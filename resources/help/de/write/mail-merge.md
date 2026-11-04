# Seriendruck, Umschläge und Etiketten
<!-- keywords: Seriendruck, zusammenführen, Sendungen, Empfänger, Adressliste, Serienbrief, Etiketten, Umschläge, Seriendruckfeld, Adressblock, Grußzeile, Regeln, Wenn Dann Sonst, Datensatz überspringen, nächster Datensatz, Vorschau, Fertig stellen und zusammenführen, Verzeichnis, E-Mail-Nachrichten, CSV, XLSX, ODS, vCard, Serienbrief, Kontakte -->

Ein **Seriendruck** erstellt viele Kopien eines Briefes, wobei in jeder Kopie die Angaben einer anderen Person stehen. Sie schreiben den Brief nur einmal. Die Angaben der Personen bewahren Sie in einer **Liste** auf. JeffWrite setzt Name und Adresse der ersten Person in die erste Kopie, die der zweiten Person in die zweite Kopie und so weiter.

Die Werkzeuge finden Sie auf der Registerkarte **Sendungen**. Für eine geführte Vorgehensweise klicken Sie auf **Sendungen > Seriendruck starten > Schritt-für-Schritt-Assistent für den Seriendruck**.

## Legen Sie fest, was Sie erstellen

Klicken Sie auf **Sendungen > Seriendruck starten** und wählen Sie eine Option:

- **Briefe**: eine Kopie der Seite für jede Person. Jede Kopie beginnt auf einer neuen Seite.
- **E-Mail-Nachrichten**: eine Nachricht für jede Person. Siehe unten „E-Mail-Nachrichten senden“.
- **Umschläge** und **Etiketten**: eine Seite dafür. Siehe die Abschnitte unten.
- **Verzeichnis**: eine Liste aller Personen ohne Seitenumbruch zwischen den Personen. Verwenden Sie das für eine Telefonliste oder einen Katalog.
- **Seriendrucktyp entfernen** macht das Dokument wieder zu einem normalen Dokument.

JeffWrite speichert die Art des Seriendrucks und den Namen der Liste in der Datei, wenn Sie sie speichern. Wenn Sie die Datei erneut öffnen, ist der Seriendruck noch vorhanden.

## Wählen Sie die Empfänger

Ein **Empfänger** ist eine Person, die eine Kopie erhält. Klicken Sie auf **Sendungen > Empfänger auswählen**:

- **Neue Liste eingeben**: Geben Sie die Personen im Fenster ein, das sich öffnet. Klicken Sie für jede Person auf **Neuer Eintrag**. JeffWrite bietet danach an, die Liste als CSV-Datei zu speichern.
- **Vorhandene Liste verwenden**: Wählen Sie eine Datei, die bereits eine Liste ist. Das kann eine CSV-Datei oder eine Textdatei mit Tabstopps sein (eine Zeile für jede Person, und die erste Zeile enthält die Spaltennamen), eine Tabellenkalkulation (.xlsx, .xls oder .ods, aus JeffCrunch oder einem anderen Programm) oder eine Kontaktdatei (.vcf).
- **Aus Kontakten auswählen**: Wählen Sie eine Kontaktdatei (.vcf). Die meisten Adressbücher und Telefone können ihre Kontakte als solche Datei speichern.

Prüfen Sie die Liste anschließend unter **Sendungen > Empfängerliste bearbeiten**:

- Entfernen Sie das Häkchen bei einer Zeile, um diese Person auszulassen.
- Klicken Sie auf eine Spaltenüberschrift, um zu sortieren.
- Geben Sie unter **Empfänger filtern** ein, um nur die Zeilen anzuzeigen, die Ihre Wörter enthalten.
- **Duplikate suchen** entfernt das Häkchen bei doppelten Einträgen.
- **Adressen überprüfen** markiert Zeilen farbig, die keine Adresse oder eine ungültig aussehende Postleitzahl haben.

## Setzen Sie die Angaben in den Brief ein

Ein **Seriendruckfeld** ist ein Platzhalter. Er zeigt den Namen einer Spalte in doppelten spitzen Klammern, etwa «Vorname». In jeder Kopie wird er durch die Angabe der jeweiligen Person ersetzt.

Klicken Sie an die Stelle, an der die Angabe stehen soll, und nutzen Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Seriendruckfeld einfügen** listet die Spalten Ihrer Liste auf. Wählen Sie eine aus.
- **Adressblock** fügt eine vollständige Postanschrift ein: Name, Firma, Straße und Ort. Wählen Sie die gewünschten Zeilen aus. Eine Zeile, die leer ist, wird weggelassen. Sie können außerdem ein Land angeben, das weggelassen werden soll, etwa Ihr eigenes. Die **Vorschau** zeigt die Adresse der ersten Person.
- **Grußzeile** fügt eine Zeile wie „Sehr geehrte Frau Dr. Fenwick,“ ein. Die Wortwahl bestimmen Sie. Die erste Auswahl, „Sehr geehrte Frau Dr. Fenwick,“, verwendet die Anrede der Person. Hat eine Person keine Anrede, werden stattdessen Vor- und Nachname verwendet, etwa „Sehr geehrte Nora Pell,“. Hat eine Person keinen Namen, lautet die Grußzeile anders, etwa „Sehr geehrte Damen und Herren,“, was Sie ändern können.
- **Zuordnen** teilt dem Adressblock und der Grußzeile mit, welche Spalte welchen Teil enthält, wenn Ihre Spaltennamen ungewöhnlich sind.

## Regeln

Eine **Regel** macht eine Kopie für manche Personen anders. Klicken Sie auf **Regeln** und wählen Sie eine Option:

- **Wenn…Dann…Sonst…**: Zeigt einen Text, wenn eine Spalte einen Wert hat, und einen anderen Text, wenn sie keinen hat. Wenn etwa Bundesland „CA“ ist, schreiben Sie „Willkommen zu Hause“.
- **Datensatz überspringen wenn…**: Lässt eine Person weg, wenn eine Spalte einen Wert hat, etwa wenn der Nachname leer ist.
- **Nächster Datensatz** und **Nächsten Datensatz wenn…**: Verwenden an derselben Stelle die Angaben der nächsten Person. Etiketten nutzen dies, um auf jedem Etikett eine andere Person einzusetzen.
- **Datensatz Nr.** fügt die Nummer der Zeile der Person in der Liste ein. **Seriendruck-Nr.** zählt die bisher erstellten Kopien.
- **Abfragen** und **Ausfüllen**: Fragt Sie beim Abschließen des Seriendrucks nach einer Angabe. „Abfragen“ speichert die Antwort unter einem Namen. „Ausfüllen“ setzt die Antwort in den Text ein.
- **Textmarke setzen**: Gibt einem Namen einen Wert. Ein Querverweis auf den Namen zeigt den Wert an.

## Zeigen Sie die Ergebnisse in der Vorschau an

Klicken Sie auf **Sendungen > Vorschau Ergebnisse**. Es öffnet sich ein Bereich, der die Seite für eine Person zeigt. Mit **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz** blättern Sie durch die Personen, oder Sie geben eine Nummer ein. **Empfänger suchen** springt zur nächsten Person, die Ihre Wörter in einer Angabe enthält.

**Seriendruckfelder hervorheben** unterlegt die Seriendruckfelder in Ihrer Seite grau, damit Sie sehen, wo sie stehen. Andere Felder, etwa Seitenzahlen, werden nicht unterlegt. Die Hervorhebung wird nur am Bildschirm angezeigt und nie gedruckt.

Ihr eigenes Dokument wird nicht verändert. **Auf Fehler prüfen** meldet Ihnen, wenn die Liste für ein Feld keine Spalte hat oder wenn niemand eingeschlossen ist.

## Schließen Sie den Seriendruck ab

Klicken Sie auf **Sendungen > Fertig stellen und zusammenführen**:

- **Einzelne Dokumente bearbeiten** erstellt ein neues Dokument mit der Kopie jeder Person. Sie können es vor dem Drucken ändern.
- **Dokumente drucken** erstellt dieses Dokument und öffnet die Seite **Drucken**.
- **Als PDF speichern** speichert alle Kopien in einer PDF-Datei.
- **E-Mail-Nachrichten senden** erstellt für jede Person eine Nachrichtendatei (.eml) in einem Ordner, den Sie auswählen. Wählen Sie die Spalte mit den E-Mail-Adressen und geben Sie einen Betreff ein. JeffWrite versendet die Nachrichten nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.

Personen, deren Häkchen Sie in der Liste entfernt haben, werden ausgelassen.

Eine Nachrichtendatei wird nur für eine Person erstellt, deren Adresse genau eine E-Mail-Adresse enthält. Enthält eine Adresse zwei (mit Komma oder Semikolon dazwischen), steht ein Name davor oder fehlt die Domain, erhält diese Person keine Nachricht, und JeffWrite teilt Ihnen die Anzahl mit. Ein langer Betreff wird in Teilen gespeichert, die E-Mail-Programme wieder zusammensetzen.

Während des Seriendrucks wird ein Fortschrittsfenster angezeigt, und mit **Abbrechen** beenden Sie ihn. Fragt eine Regel nach einer Angabe (Abfragen oder Ausfüllen), werden Sie für jede verschiedene Frage einmal gefragt, höchstens 20 Fragen. Danach verwenden die übrigen Kopien ihre Standardantworten. Wenn Sie eine Frage abbrechen, wird der gesamte Seriendruck beendet. Der Seriendruck stoppt und weist darauf hin, wenn er mehr als 10.000 Seiten erzeugen würde. So dauert ein sehr langes Dokument mit einer sehr langen Liste nicht stundenlang. Führen Sie in diesem Fall einen Teil der Liste nach dem anderen zusammen.

## Umschläge

Klicken Sie auf **Sendungen > Umschläge**. Geben Sie die Empfängeradresse und Ihre Absenderadresse ein und wählen Sie die Größe des Umschlags aus der Liste. Die Größen sind nach Breite und Höhe benannt. Klicken Sie auf **Zum Dokument hinzufügen**, um den Umschlag auf eine eigene Seite am Anfang Ihres Dokuments zu setzen, oder auf **Drucken**, um ihn zu drucken. Die Seite ist so breit wie die lange Seite des Umschlags, so wie die meisten Drucker Umschläge einziehen.

Um Umschläge für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Umschläge**. Jeder Umschlag enthält einen Adressblock.

## Etiketten

Klicken Sie auf **Sendungen > Etiketten**. Geben Sie die Adresse ein und wählen Sie den Bogen aus der Liste. Ein Bogen wird nach der Anzahl seiner Etiketten und der Größe jedes Etiketts benannt. Ist Ihr Bogen nicht in der Liste, wählen Sie **Eigener Bogen** und geben Sie die Maße ein: wie viele Etiketten nebeneinander und untereinander stehen, wie groß ein Etikett ist, den Abstand oben und seitlich vom ersten Etikett und den Abstand zwischen den Etiketten.

- **Ganze Seite mit demselben Etikett** setzt auf jedes Etikett des Bogens dieselbe Adresse.
- **Einzelnes Etikett** setzt die Adresse auf ein einziges Etikett, damit Sie einen teilweise benutzten Bogen aufbrauchen können. Geben Sie Zeile und Spalte an.

Klicken Sie auf **Neues Dokument**, um ein Dokument zu erstellen, das Sie ansehen und ändern können, oder auf **Drucken**, um es zu drucken.

Um Etiketten für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Etiketten**. Das erste Etikett enthält einen Adressblock, und jedes weitere Etikett beginnt mit einer Regel **Nächster Datensatz**. Wenn Sie das erste Etikett ändern, klicken Sie auf **Etiketten aktualisieren**, um die Änderung auf alle anderen zu übertragen.

## Was die Datei enthält

Die Art des Seriendrucks, der Name und der Ort der Liste, der Betreff und die E-Mail-Spalte werden in Ihrer .docx-Datei gespeichert. Welche Personen Sie abgewählt haben und Ihre Auswahl unter Zuordnen werden nicht gespeichert. Sie werden nach dem Öffnen der Datei erneut abgefragt. Eine Liste wird bei jedem Öffnen aus ihrer Datei neu gelesen. Liegt die Datei nicht mehr am alten Ort, sucht JeffWrite zunächst neben Ihrem Dokument und fragt Sie anschließend, die Datei auszuwählen.

Eine Liste kann insgesamt bis zu 1.000 Spalten und 4.000.000 Zellen haben. Ein Seriendruck verwendet die ersten 100.000 Personen. Hat eine Datei mehr, verwendet JeffWrite, was hineinpasst, und teilt Ihnen das mit. Zeilen ohne Inhalt werden aus der Liste ausgelassen.

## Siehe auch

- [Felder verwenden](fields)
- [Drucken](print)
- [Tabellen erstellen und ändern](tables)
