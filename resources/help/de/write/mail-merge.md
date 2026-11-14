# Seriendruck, Umschläge und Etiketten
<!-- keywords: Seriendruck, zusammenführen, Sendungen, Empfänger, Adressliste, Serienbrief, Etiketten, Umschläge, Seriendruckfeld, Adressblock, Grußzeile, Regeln, Wenn Dann Sonst, Datensatz überspringen, nächster Datensatz, Vorschau Ergebnisse, Fertig stellen und zusammenführen, Verzeichnis, E-Mail-Nachrichten, CSV, XLSX, ODS, vCard, Kontakte, Barcode, Serienbrief-Assistent -->

Ein **Seriendruck** erstellt viele Kopien eines Briefs, und in jeder steht die Adresse einer anderen Person. Sie schreiben den Brief einmal. Die Angaben zu den Personen bewahren Sie in einer **Liste** auf. JeffWrite setzt den Namen und die Adresse der ersten Person in die erste Kopie, die der zweiten Person in die zweite Kopie und so weiter.

Die Werkzeuge finden Sie auf der Registerkarte **Sendungen**. Für eine geführte Vorgehensweise klicken Sie auf **Sendungen > Seriendruck starten > Schritt-für-Schritt-Seriendruck-Assistent**.

## Wählen Sie, was Sie erstellen

Klicken Sie auf **Sendungen > Seriendruck starten** und wählen Sie eine Option:

- **Briefe**: eine Kopie der Seite für jede Person. Jede Kopie beginnt auf einer neuen Seite.
- **E-Mail-Nachrichten**: eine Nachricht für jede Person. Siehe unten „E-Mail-Nachrichten senden“.
- **Umschläge** und **Etiketten**: eine Seite dafür. Siehe die Abschnitte unten.
- **Verzeichnis**: eine Liste aller Personen ohne Seitenumbruch zwischen den Personen. Verwenden Sie sie für eine Telefonliste oder einen Katalog.
- **Seriendrucktyp entfernen** macht das Dokument wieder zu einem normalen Dokument.

JeffWrite speichert die Art des Seriendrucks und den Namen der Liste in der Datei, wenn Sie sie speichern. Öffnen Sie die Datei später erneut, ist der Seriendruck noch vorhanden.

## Wählen Sie Ihre Empfänger

Ein **Empfänger** ist eine Person, die eine Kopie erhält. Klicken Sie auf **Sendungen > Empfänger auswählen**:

- **Neue Liste eingeben**: Geben Sie die Personen in das Fenster ein, das sich öffnet. Klicken Sie für jede Person auf **Neuer Eintrag**. JeffWrite bietet danach an, die Liste als .csv-Datei zu speichern.
- **Vorhandene Liste verwenden**: Wählen Sie eine Datei, die bereits eine Liste ist. Das kann eine .csv-Datei oder eine Textdatei mit Tabstopps sein (eine Zeile für jede Person, und die erste Zeile enthält die Spaltennamen), eine Tabellenkalkulation (.xlsx, .xls oder .ods, aus JeffCrunch oder einem anderen Programm) oder eine Kontaktdatei (.vcf).
- **Aus Kontakten auswählen**: Wählen Sie eine Kontaktdatei (.vcf). Die meisten Adressbücher und Telefone können ihre Kontakte als eine solche Datei speichern.

Klicken Sie danach auf **Sendungen > Empfängerliste bearbeiten**, um die Liste zu prüfen:

- Entfernen Sie das Häkchen bei einer Zeile, um diese Person auszulassen.
- Klicken Sie auf eine Spaltenüberschrift, um zu sortieren.
- Geben Sie unter **Empfänger filtern** ein, um nur die Zeilen anzuzeigen, die Ihre Wörter enthalten.
- **Duplikate suchen** entfernt das Häkchen bei doppelten Einträgen.
- **Adressen überprüfen** markiert die Zeilen farbig, die keine Adresse oder eine augenscheinlich falsche Postleitzahl haben.

## Setzen Sie die Angaben in den Brief ein

Ein **Seriendruckfeld** ist ein Platzhalter. Es zeigt den Namen einer Spalte in doppelten spitzen Anführungszeichen, etwa «Vorname». In jeder Kopie wird es durch die Angabe der jeweiligen Person ersetzt.

Klicken Sie an die Stelle, an der die Angabe stehen soll, und verwenden Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Seriendruckfeld einfügen** listet die Spalten Ihrer Liste auf. Wählen Sie eine aus.
  - **Barcode aus Feld** fügt in derselben Liste einen Barcode ein, dessen Wert eine Spalte ist, etwa eine Artikelnummer oder die 13-stellige ISBN eines Buchs. Wählen Sie die Spalte, dann den Typ, und sehen Sie sich das Fenster **Barcode einfügen** an (was Sie dort eingeben, ist nur ein Beispiel). Das Fenster bietet nur, was ein Feld enthalten kann: den Typ, die Größe, die Balkenhöhe, die Ziffern und die QR-Stufe. Es lässt den Buchpreis, einen Zusatzcode, eine Prüfziffer und den hellen Hintergrund weg, was ein Feld nicht angeben kann; eine ISBN eines Buchs ist ein EAN-13. Im Dokument wird das Feld bis zum Seriendruck als Beispiel des Typs als Barcode dargestellt. Um Typ oder Aussehen zu ändern, doppelklicken Sie darauf oder klicken Sie es mit der rechten Maustaste an und wählen **Barcode bearbeiten**. Beim Seriendruck erhält die Kopie jeder Person einen Barcode mit deren eigenem Wert, als Bild in die Zeile gezeichnet. So wird er wie jedes Bild mitgedruckt und in PDF gespeichert. Der Wert ist nur Daten: Anführungszeichen oder ein umgekehrter Schrägstrich darin ändern nichts. Ist ein Wert leer, entsteht kein Barcode. Kann ein Code den Wert nicht aufnehmen (ein Buchstabe in einer EAN-13 oder mehr als 500 Zeichen in Code 128), zeigt die Kopie an dieser Stelle einen kurzen Hinweis in eckigen Klammern, und die übrigen Kopien sind nicht betroffen. Ein Seriendruck erstellt höchstens 2.000 Barcodes, wobei auch jeder Wert mitzählt, der nicht dargestellt werden kann. Wenn es zu viele sind, drucken Sie für weniger Personen. In einer gespeicherten `.docx`-Datei ist das Feld ein DISPLAYBARCODE-Feld mit der Spalte darin, sodass es beim erneuten Öffnen erhalten bleibt. Ein Feld mit einem eingetippten Wert oder einem Typ, den JeffWrite nicht zeichnet (zum Beispiel UPC-E), bleibt unverändert. Bei E-Mail-Serienbriefen wird der Wert als Text eingefügt.
- **Adressblock** fügt eine vollständige Postanschrift ein: Name, Firma, Straße und Ort. Wählen Sie die gewünschten Zeilen aus. Eine Zeile ohne Inhalt wird weggelassen. Sie können auch ein Land angeben, das weggelassen werden soll, etwa Ihr eigenes. Die **Vorschau** zeigt die Adresse der ersten Person.
- **Grußzeile** fügt eine Zeile wie „Sehr geehrte Frau Dr. Fenwick,“ ein. Die Formulierung wählen Sie selbst. Die erste Auswahl, „Sehr geehrte Frau Dr. Fenwick,“, verwendet den Titel der Person. Hat eine Person keinen Titel, nimmt die Grußzeile stattdessen Vor- und Nachnamen, wie in „Sehr geehrte Nora Pell,“. Hat eine Person keinen Namen, lautet die Grußzeile anders, etwa „Sehr geehrte Damen und Herren,“, was Sie ändern können.
- **Zuordnen** teilt dem Adressblock und der Grußzeile mit, welche Spalte welchen Teil enthält, wenn Ihre Spaltennamen ungewöhnlich sind.

## Regeln

Eine **Regel** macht eine Kopie für einige Personen anders. Klicken Sie auf **Regeln** und wählen Sie eine aus:

- **Wenn…Dann…Sonst**: Zeigt einen Text, wenn eine Spalte einen Wert enthält, und einen anderen Text, wenn sie keinen enthält. Wenn etwa das Bundesland „CA“ ist, schreiben Sie „Willkommen zu Hause“.
- **Datensatz überspringen falls**: Lässt eine Person aus, wenn eine Spalte einen Wert enthält, etwa wenn der Nachname leer ist.
- **Nächster Datensatz** und **Nächsten Datensatz falls**: Verwenden Sie die Angaben der nächsten Person an derselben Stelle. Etiketten nutzen dies, um auf jedem Etikett eine andere Person einzusetzen.
- **Datensatz Nr.** fügt die Nummer der Zeile der Person in der Liste ein. **Seriendrucknummer** zählt die bisher erstellten Kopien.
- **Abfragen** und **Ausfüllfeld**: Fragen Sie beim Abschluss des Seriendrucks nach etwas. Abfragen speichert die Antwort unter einem Namen. Ausfüllfeld setzt die Antwort in den Text ein.
- **Textmarke setzen**: Gibt einem Namen einen Wert. Ein Querverweis auf den Namen zeigt den Wert an.

## Ergebnisse in der Vorschau anzeigen

Klicken Sie auf **Sendungen > Vorschau Ergebnisse**. Es öffnet sich ein Bereich, der die Seite für eine Person zeigt. Mit **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz** blättern Sie durch die Personen, oder Sie geben eine Nummer ein. **Empfänger suchen** springt zur nächsten Person, die Ihre Wörter in einer Angabe enthält.

**Seriendruckfelder hervorheben** unterlegt die Seriendruckfelder auf Ihrer Seite grau, damit Sie sehen, wo sie stehen. Andere Felder, etwa Seitenzahlen, werden nicht unterlegt. Die Hervorhebung erscheint nur auf dem Bildschirm und wird nie gedruckt.

Ihr eigenes Dokument wird dabei nicht verändert. **Auf Fehler prüfen** meldet, wenn die Liste für ein Feld keine Spalte hat oder wenn niemand enthalten ist.

## Seriendruck abschließen

Klicken Sie auf **Sendungen > Fertig stellen und zusammenführen**:

- **Einzelne Dokumente bearbeiten** erstellt ein neues Dokument mit der Kopie jeder Person. Sie können es vor dem Drucken noch ändern.
- **Dokumente drucken** erstellt dieses Dokument und öffnet die Seite **Drucken**.
- **Als PDF speichern** speichert alle Kopien in einer PDF-Datei.
- **E-Mail-Nachrichten senden** erstellt für jede Person eine .eml-Nachrichtendatei in einem Ordner, den Sie wählen. Wählen Sie die Spalte mit den E-Mail-Adressen und geben Sie einen Betreff ein. JeffWrite versendet die Nachrichten nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.

Personen, bei denen Sie das Häkchen in der Liste entfernt haben, werden ausgelassen.

Eine Nachrichtendatei wird nur für eine Person erstellt, deren Adresse genau eine E-Mail-Adresse enthält. Enthält eine Adresse zwei (mit Komma oder Semikolon dazwischen), steht ein Name davor oder fehlt die Domain, erhält diese Person keine Nachricht, und JeffWrite teilt Ihnen mit, wie viele es sind. Ein langer Betreff wird in Teilen gespeichert, die E-Mail-Programme wieder zusammensetzen.

Während des Seriendrucks zeigt ein Fortschrittsfenster den Stand an, mit **Abbrechen** halten Sie den Vorgang an. Stellt eine Regel eine Frage (Abfragen oder Ausfüllfeld), werden Sie zu jeder unterschiedlichen Frage einmal gefragt, höchstens 20-mal. Danach verwenden die übrigen Kopien ihre Standardantworten. Brechen Sie eine Frage ab, stoppt der ganze Seriendruck. Würde ein Seriendruck mehr als 10.000 Seiten erzeugen, wird er mit einem Hinweis angehalten, damit ein sehr langes Dokument mit einer sehr langen Liste nicht stundenlang läuft. Führen Sie dann einen Teil der Liste nach dem anderen zusammen.

## Umschläge

Klicken Sie auf **Sendungen > Umschläge**. Geben Sie die Empfängeradresse und Ihre Absenderadresse ein und wählen Sie die Umschlaggröße aus der Liste. Die Größen sind nach Breite und Höhe benannt. Klicken Sie auf **Zum Dokument hinzufügen**, um den Umschlag auf einer eigenen Seite am Anfang Ihres Dokuments einzufügen, oder auf **Drucken**, um ihn zu drucken. Die Seite ist so breit wie die lange Seite des Umschlags, so wie die meisten Drucker Umschläge einziehen.

Um Umschläge für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Umschläge**. Jeder Umschlag enthält einen Adressblock.

## Etiketten

Klicken Sie auf **Sendungen > Etiketten**. Geben Sie die Adresse ein und wählen Sie den Bogen aus der Liste. Ein Bogen wird nach der Anzahl seiner Etiketten und der Größe jedes Etiketts benannt. Ist Ihr Bogen nicht in der Liste, wählen Sie **Eigener Bogen** und geben Sie die Maße ein: wie viele Etiketten nebeneinander und untereinander stehen, wie groß ein Etikett ist, den Abstand oberhalb und seitlich des ersten Etiketts sowie den Abstand der Etiketten voneinander.

- **Ganze Seite mit demselben Etikett** setzt auf jedes Etikett des Bogens dieselbe Adresse.
- **Einzelnes Etikett** setzt die Adresse auf ein einziges Etikett, damit Sie einen teilweise benutzten Bogen aufbrauchen können. Geben Sie Zeile und Spalte an.

Klicken Sie auf **Neues Dokument**, um ein Dokument zu erstellen, das Sie ansehen und ändern können, oder auf **Drucken**, um es zu drucken.

Um Etiketten für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Etiketten**. Das erste Etikett enthält einen Adressblock, und jedes weitere Etikett beginnt mit einer Regel **Nächster Datensatz**. Wenn Sie das erste Etikett ändern, klicken Sie auf **Etiketten aktualisieren**, um die Änderung auf alle übrigen zu übertragen.

## Was die Datei enthält

Die Art des Seriendrucks, Name und Speicherort der Liste, der Betreff und die E-Mail-Spalte werden in Ihrer .docx-Datei gespeichert. Welche Personen Sie abgewählt haben und Ihre Einstellungen unter Zuordnen werden nicht gespeichert. Sie werden nach dem Öffnen der Datei erneut abgefragt. Eine Liste wird bei jedem Öffnen aus ihrer Datei neu gelesen. Befindet sich die Datei nicht mehr dort, wo sie war, sucht JeffWrite zuerst neben Ihrem Dokument und fragt Sie dann, die Datei auszuwählen.

Eine Liste kann insgesamt bis zu 1.000 Spalten und 4.000.000 Zellen haben; ein Seriendruck verwendet die ersten 100.000 Personen. Enthält eine Datei mehr, verwendet JeffWrite, was hineinpasst, und teilt Ihnen das mit. Zeilen ohne Inhalt werden aus der Liste ausgelassen.

## Siehe auch

- [Felder verwenden](fields)
- [Drucken](print)
- [Tabellen erstellen und bearbeiten](tables)
