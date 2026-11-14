# Seriendruck, Umschläge und Etiketten
<!-- keywords: Seriendruck, zusammenführen, Sendungen, Empfänger, Adressliste, Serienbrief, Etiketten, Umschläge, Seriendruckfeld, Adressblock, Grußzeile, Regeln, Wenn Dann Sonst, Datensatz überspringen, nächster Datensatz, Vorschau Ergebnisse, Fertig stellen und zusammenführen, Verzeichnis, E-Mail-Nachrichten, CSV, xlsx, ods, vCard, Serienbrief, Massenbrief, Adressen, Kontakte -->

Ein **Seriendruck** erstellt viele Kopien eines Briefes, wobei in jeder Kopie die Angaben einer anderen Person stehen. Sie schreiben den Brief einmal. Die Angaben der Personen bewahren Sie in einer **Liste** auf. JeffWrite setzt den Namen und die Anschrift der ersten Person in die erste Kopie, die der zweiten Person in die zweite Kopie und so weiter.

Die Werkzeuge finden Sie auf der Registerkarte **Sendungen**. Für eine geführte Vorgehensweise klicken Sie auf **Sendungen > Seriendruck starten > Schritt-für-Schritt-Assistent für den Seriendruck**.

## Art festlegen

Klicken Sie auf **Sendungen > Seriendruck starten** und wählen Sie eine Art:

- **Briefe**: eine Kopie der Seite für jede Person. Jede Kopie beginnt auf einer neuen Seite.
- **E-Mail-Nachrichten**: eine Nachricht für jede Person. Siehe unten „E-Mail-Nachrichten senden“.
- **Umschläge** und **Etiketten**: eine Seite dafür. Siehe die Abschnitte unten.
- **Verzeichnis**: eine Liste aller Personen ohne Seitenwechsel zwischen den Personen. Verwenden Sie sie für eine Telefonliste oder einen Katalog.
- **Seriendrucktyp entfernen** macht das Dokument wieder zu einem gewöhnlichen Dokument.

JeffWrite speichert die Art des Seriendrucks und den Namen der Liste in der Datei, wenn Sie sie speichern. Beim erneuten Öffnen ist der Seriendruck noch vorhanden.

## Empfänger auswählen

Ein **Empfänger** ist eine Person, die eine Kopie erhält. Klicken Sie auf **Sendungen > Empfänger auswählen**:

- **Neue Liste eingeben**: Geben Sie die Personen in das Fenster ein, das sich öffnet. Klicken Sie für jede Person auf **Neuer Eintrag**. JeffWrite bietet danach an, die Liste als CSV-Datei zu speichern.
- **Vorhandene Liste verwenden**: Wählen Sie eine Datei, die bereits eine Liste ist. Das kann eine CSV-Datei oder eine Textdatei mit Tabstopps sein (eine Zeile für jede Person, und die erste Zeile enthält die Spaltennamen), eine Tabellenkalkulation (.xlsx, .xls oder .ods aus JeffCrunch oder einem anderen Programm) oder eine Kontaktdatei (.vcf).
- **Aus Kontakten auswählen**: Wählen Sie eine Kontaktdatei (.vcf). Die meisten Adressbücher und Telefone können ihre Kontakte als solche Datei speichern.

Klicken Sie danach auf **Sendungen > Empfängerliste bearbeiten**, um die Liste zu prüfen:

- Entfernen Sie das Häkchen bei einer Zeile, um diese Person auszulassen.
- Klicken Sie auf eine Spaltenüberschrift, um zu sortieren.
- Geben Sie in **Empfänger filtern** etwas ein, um nur die Zeilen anzuzeigen, die Ihre Wörter enthalten.
- **Duplikate suchen** entfernt das Häkchen bei doppelten Einträgen.
- **Adressen überprüfen** färbt die Zeilen ein, denen eine Adresse fehlt oder deren Postleitzahl falsch aussieht.

## Angaben in den Brief einfügen

Ein **Seriendruckfeld** ist ein Platzhalter. Es zeigt den Namen einer Spalte in doppelten spitzen Anführungszeichen, etwa «Vorname». In jeder Kopie wird es durch die Angabe der jeweiligen Person ersetzt.

Klicken Sie an die Stelle, an der die Angabe stehen soll, und verwenden Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Seriendruckfeld einfügen** listet die Spalten Ihrer Liste auf. Wählen Sie eine aus.
  - **Barcode aus Feld** steht in derselben Liste und fügt einen Barcode ein, dessen Wert eine Spalte ist, etwa eine Artikelnummer oder die 13-stellige ISBN eines Buches. Wählen Sie die Spalte, dann den Typ, und sehen Sie im Fenster **Barcode einfügen** nach (was Sie dort eingeben, ist nur ein Beispiel). Das Fenster bietet nur, was ein Feld tragen kann: den Typ, die Größe, die Balkenhöhe, die Ziffern und die QR-Stufe. Es lässt den Buchpreis, einen Zusatzcode, eine Prüfziffer und den hellen Hintergrund aus, was ein Feld nicht angeben kann; die ISBN eines Buches ist ein EAN-13. Im Dokument wird das Feld als Barcode gezeichnet, bis zum Seriendruck als Beispiel des Typs. Um Typ oder Aussehen zu ändern, doppelklicken Sie darauf oder klicken Sie mit der rechten Maustaste und wählen **Barcode bearbeiten**. Beim Seriendruck erhält die Kopie jeder Person einen Barcode mit deren eigenem Wert, als Bild in der Zeile gezeichnet. So wird er wie jedes Bild gedruckt und in PDF gespeichert. Der Wert ist nur Daten: Anführungszeichen oder ein umgekehrter Schrägstrich darin ändern nichts. Ist ein Wert leer, entsteht kein Barcode. Kann ein Code einen Wert nicht aufnehmen (ein Buchstabe in einem EAN-13 oder mehr als 500 Zeichen in Code 128), steht an dessen Stelle in der Kopie eine kurze Meldung in eckigen Klammern; die übrigen Kopien sind davon nicht betroffen. Ein Seriendruck erzeugt höchstens 2.000 Barcodes, wobei jeder Wert zählt, der nicht dargestellt werden kann. Meldet das Programm, dass es zu viele sind, führen Sie den Seriendruck für weniger Personen aus. In einer gespeicherten `.docx`-Datei ist das Feld ein DISPLAYBARCODE-Feld mit der Spalte darin, sodass es beim Speichern und erneuten Öffnen erhalten bleibt. Ein Feld mit eingetragenem Wert oder mit einem Typ, den JeffWrite nicht zeichnet (UPC-E zum Beispiel), bleibt unverändert. Bei E-Mail-Serienläufen wird der Wert als Text eingefügt.
- **Adressblock** fügt eine vollständige Postanschrift ein: Name, Firma, Straße und Ort. Wählen Sie die gewünschten Zeilen aus. Eine Zeile ohne Inhalt wird weggelassen. Sie können außerdem ein Land angeben, das weggelassen werden soll, zum Beispiel Ihr eigenes. Die **Vorschau** zeigt die Anschrift der ersten Person.
- **Grußzeile** fügt eine Zeile wie „Sehr geehrte Frau Dr. Fenwick,“ ein. Die Formulierung wählen Sie selbst. Die erste Auswahl, „Sehr geehrte Frau Dr. Fenwick,“, verwendet den Titel der Person. Hat eine Person keinen Titel, verwendet sie stattdessen Vor- und Nachnamen, wie in „Liebe Nora Pell,“. Hat eine Person keinen Namen, lautet die Anrede anders, zum Beispiel „Sehr geehrte Damen und Herren,“, und Sie können sie ändern.
- **Feldzuordnung** teilt dem Adressblock und der Grußzeile mit, welche Spalte welchen Teil enthält, wenn Ihre Spaltennamen ungewöhnlich sind.

## Regeln

Eine **Regel** macht eine Kopie für manche Personen anders. Klicken Sie auf **Regeln** und wählen Sie eine:

- **Wenn…Dann…Sonst**: Zeigt einen Text, wenn eine Spalte einen Wert hat, und einen anderen Text, wenn sie keinen hat. Wenn zum Beispiel das Bundesland „CA“ ist, schreiben Sie „Willkommen zu Hause“.
- **Datensatz überspringen wenn**: Lässt eine Person aus, wenn eine Spalte einen Wert hat, etwa wenn der Nachname leer ist.
- **Nächster Datensatz** und **Nächsten Datensatz wenn…**: Verwendet die Angaben der nächsten Person an derselben Stelle. Etiketten nutzen dies, um auf jedem Etikett eine andere Person einzusetzen.
- **Datensatz-Nr.** fügt die Nummer der Zeile der Person in der Liste ein. **Seriendruck-Nr.** zählt die bisher erstellten Kopien.
- **Abfragen** und **Ausfüllfeld**: Fragen Sie beim Abschließen des Seriendrucks nach. „Abfragen“ speichert die Antwort unter einem Namen. „Ausfüllfeld“ setzt die Antwort in den Text ein.
- **Textmarke setzen**: Gibt einem Namen einen Wert. Ein Querverweis auf den Namen zeigt den Wert an.

## Ergebnisse in der Vorschau ansehen

Klicken Sie auf **Sendungen > Vorschau Ergebnisse**. Ein Bereich öffnet sich und zeigt die Seite für eine Person. Mit **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz** blättern Sie durch die Personen, oder Sie geben eine Nummer ein. **Empfänger suchen** springt zur nächsten Person, deren Angaben Ihr Wort enthalten.

**Seriendruckfelder hervorheben** unterlegt die Seriendruckfelder auf Ihrer Seite grau, damit Sie sehen, wo sie stehen. Andere Felder, etwa Seitenzahlen, werden nicht unterlegt. Die Unterlegung erscheint nur auf dem Bildschirm und wird nie gedruckt.

Ihr eigenes Dokument wird dabei nicht verändert. **Fehler überprüfen** meldet Ihnen, wenn die Liste für ein Feld keine Spalte hat oder wenn niemand eingeschlossen ist.

## Seriendruck abschließen

Klicken Sie auf **Sendungen > Fertig stellen und zusammenführen**:

- **Einzelne Dokumente bearbeiten** erstellt ein neues Dokument mit der Kopie jeder Person. Sie können es vor dem Drucken noch ändern.
- **Dokumente drucken** erstellt dieses Dokument und öffnet die Seite **Drucken**.
- **Als PDF speichern** speichert alle Kopien in einer PDF-Datei.
- **E-Mail-Nachrichten senden** erstellt für jede Person eine Nachrichtendatei (.eml) in einem Ordner, den Sie wählen. Wählen Sie die Spalte mit den E-Mail-Adressen und geben Sie einen Betreff ein. JeffWrite versendet die Nachrichten nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.

Personen, deren Häkchen Sie in der Liste entfernt haben, werden ausgelassen.

Eine Nachrichtendatei entsteht nur für eine Person, deren Adresse genau eine E-Mail-Adresse enthält. Enthält eine Adresse zwei (mit Komma oder Semikolon dazwischen), steht ein Name davor oder fehlt die Domain, erhält diese Person keine Nachricht, und JeffWrite teilt Ihnen die Anzahl mit. Ein langer Betreff wird in Teilen gespeichert, die E-Mail-Programme wieder zusammensetzen.

Ein Seriendruck zeigt ein Fortschrittsfenster, und **Abbrechen** hält ihn an. Fragt eine Regel etwas ab (Abfragen oder Ausfüllfeld), werden Sie für jede unterschiedliche Frage einmal gefragt, höchstens 20-mal. Danach verwenden die übrigen Kopien ihre Standardantworten. Brechen Sie eine Frage ab, wird der gesamte Seriendruck angehalten. Ein Seriendruck wird angehalten und dies gemeldet, wenn er mehr als 10.000 Seiten erzeugen würde, damit ein sehr langes Dokument mit einer sehr langen Liste nicht stundenlang läuft. Führen Sie den Seriendruck in diesem Fall für einen Teil der Liste nacheinander aus.

## Umschläge

Klicken Sie auf **Sendungen > Umschläge**. Geben Sie die Empfängeradresse und Ihre Absenderadresse ein und wählen Sie die Größe des Umschlags aus der Liste. Die Größen sind nach Breite und Höhe benannt. Klicken Sie auf **Zum Dokument hinzufügen**, um den Umschlag auf eine eigene Seite am Anfang Ihres Dokuments zu setzen, oder auf **Drucken**, um ihn zu drucken. Die Seite ist so breit wie die lange Seite des Umschlags, so wie die meisten Drucker Umschläge einziehen.

Um Umschläge für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Umschläge**. Jeder Umschlag enthält einen Adressblock.

## Etiketten

Klicken Sie auf **Sendungen > Etiketten**. Geben Sie die Adresse ein und wählen Sie den Bogen aus der Liste. Ein Bogen wird nach der Anzahl seiner Etiketten und der Größe jedes Etiketts benannt. Ist Ihr Bogen nicht in der Liste, wählen Sie **Eigener Bogen** und geben Sie die Maße ein: wie viele Etiketten nebeneinander und untereinander stehen, wie groß ein Etikett ist, den Abstand oberhalb und seitlich des ersten Etiketts und wie weit die Etiketten voneinander entfernt sind.

- **Ganze Seite mit demselben Etikett** setzt auf jedes Etikett des Bogens dieselbe Adresse.
- **Einzelnes Etikett** setzt die Adresse auf ein einziges Etikett, sodass Sie einen teilweise benutzten Bogen aufbrauchen können. Geben Sie Zeile und Spalte an.

Klicken Sie auf **Neues Dokument**, um ein Dokument zu erstellen, das Sie ansehen und ändern können, oder auf **Drucken**, um es zu drucken.

Um Etiketten für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Etiketten**. Das erste Etikett enthält einen Adressblock, und jedes weitere Etikett beginnt mit einer Regel **Nächster Datensatz**. Wenn Sie das erste Etikett ändern, klicken Sie auf **Etiketten aktualisieren**, um die Änderung auf alle anderen zu übertragen.

## Was die Datei enthält

Die Art des Seriendrucks, der Name und Ort der Liste, der Betreff und die E-Mail-Spalte werden in Ihrer .docx-Datei gespeichert. Welche Personen Sie abgewählt haben und Ihre Auswahl bei der Feldzuordnung werden nicht gespeichert. Danach werden sie beim erneuten Öffnen wieder abgefragt. Eine Liste wird bei jedem Öffnen aus ihrer Datei neu gelesen. Befindet sich die Datei nicht mehr dort, sucht JeffWrite neben Ihrem Dokument und fragt Sie dann, die Datei auszuwählen.

Eine Liste kann insgesamt bis zu 1.000 Spalten und 4.000.000 Zellen haben, und ein Seriendruck verwendet die ersten 100.000 Personen. Enthält eine Datei mehr, verwendet JeffWrite, was hineinpasst, und teilt Ihnen das mit. Zeilen ohne Inhalt werden aus der Liste ausgelassen.

## Siehe auch

- [Felder verwenden](fields)
- [Drucken](print)
- [Tabellen erstellen und ändern](tables)
