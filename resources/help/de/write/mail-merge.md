# Seriendruck, Umschläge und Etiketten
<!-- keywords: Seriendruck, zusammenführen, Sendungen, Empfänger, Adressliste, Serienbrief, Etiketten, Umschläge, Seriendruckfeld, Adressblock, Grußzeile, Regeln, Wenn Dann Sonst, Datensatz überspringen, nächster Datensatz, Ergebnisse Vorschau, Fertig stellen und zusammenführen, Verzeichnis, E-Mail-Nachrichten, csv, xlsx, ods, vcard, Kontakte, Barcode -->

Ein **Seriendruck** erstellt viele Kopien eines Briefs, und in jeder steht die Adresse einer anderen Person. Sie schreiben den Brief einmal. Die Angaben zu den Personen bewahren Sie in einer **Liste** auf. JeffWrite setzt den Namen und die Adresse der ersten Person in die erste Kopie, die der zweiten Person in die zweite Kopie und so weiter.

Die Werkzeuge finden Sie auf der Registerkarte **Sendungen**. Für eine Schritt-für-Schritt-Anleitung klicken Sie auf **Sendungen > Seriendruck starten > Schritt-für-Schritt-Assistent für den Seriendruck**.

## Wählen Sie, was Sie erstellen

Klicken Sie auf **Sendungen > Seriendruck starten** und wählen Sie eine Option:

- **Briefe**: eine Kopie der Seite für jede Person. Jede Kopie beginnt auf einer neuen Seite.
- **E-Mail-Nachrichten**: eine Nachricht für jede Person. Siehe „E-Mail-Nachrichten senden“ weiter unten.
- **Umschläge** und **Etiketten**: eine Seite, die dafür angelegt wird. Siehe die Abschnitte unten.
- **Verzeichnis**: eine Liste aller Personen ohne Seitenumbruch zwischen den Personen. Verwenden Sie das für eine Telefonliste oder einen Katalog.
- **Seriendrucktyp entfernen** macht aus dem Dokument wieder ein gewöhnliches Dokument.

JeffWrite speichert die Art des Seriendrucks und den Namen der Liste in der Datei, wenn Sie sie speichern. Öffnen Sie die Datei später erneut, ist der Seriendruck weiterhin vorhanden.

## Wählen Sie Ihre Empfänger

Ein **Empfänger** ist eine Person, die eine Kopie erhält. Klicken Sie auf **Sendungen > Empfänger auswählen**:

- **Neue Liste eingeben**: Geben Sie die Personen in das Fenster ein, das sich öffnet. Klicken Sie für jede Person auf **Neuer Eintrag**. JeffWrite bietet dann an, die Liste als .csv-Datei zu speichern.
- **Vorhandene Liste verwenden**: Wählen Sie eine Datei, die bereits eine Liste ist. Das kann eine .csv-Datei oder eine Textdatei mit Tabstopps sein (eine Zeile für jede Person, und die erste Zeile enthält die Spaltennamen), eine Tabellenkalkulation (.xlsx, .xls oder .ods, aus JeffCrunch oder einem anderen Programm) oder eine Kontaktdatei (.vcf).
- **Aus Kontakten auswählen**: Wählen Sie eine Kontaktdatei (.vcf). Die meisten Adressbücher und Telefone können ihre Kontakte als eine solche Datei speichern.

Klicken Sie danach auf **Sendungen > Empfängerliste bearbeiten**, um die Liste zu prüfen:

- Entfernen Sie das Häkchen bei einer Zeile, um diese Person auszulassen.
- Klicken Sie auf eine Spaltenüberschrift, um zu sortieren.
- Geben Sie unter **Empfänger filtern** ein Wort ein, damit nur die Zeilen angezeigt werden, in denen dieses Wort vorkommt.
- **Duplikate suchen** entfernt das Häkchen bei Wiederholungen.
- **Adressen überprüfen** färbt die Zeilen ein, die keine Adresse oder eine merkwürdig aussehende Postleitzahl haben.

## Setzen Sie die Angaben in den Brief ein

Ein **Seriendruckfeld** ist ein Platzhalter. Es zeigt den Namen einer Spalte in doppelten spitzen Klammern, etwa «Vorname». In jeder Kopie wird es durch die Angabe der jeweiligen Person ersetzt.

Klicken Sie an die Stelle, an der die Angabe stehen soll, und verwenden Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Seriendruckfeld einfügen** listet die Spalten Ihrer Liste auf. Wählen Sie eine aus.
  - **Barcode aus Feld** steht in derselben Liste und fügt einen Barcode ein, dessen Wert eine Spalte ist, zum Beispiel eine Artikelnummer oder die 13-stellige ISBN eines Buchs. Wählen Sie die Spalte, dann Typ, und sehen Sie im Fenster **Barcode einfügen** nach (was Sie dort eingeben, ist nur ein Beispiel). Das Fenster bietet nur, was ein Feld tragen kann: den Typ, die Größe, die Balkenhöhe, die Ziffern und die QR-Stufe. Es lässt den Buchpreis, einen Zusatzcode, ein Prüfzeichen und den hellen Hintergrund weg, was ein Feld nicht ausdrücken kann; die ISBN eines Buchs ist ein EAN-13. Im Dokument wird das Feld als Barcode dargestellt, bis zum Seriendruck als Muster des Typs. Um Typ oder Aussehen zu ändern, doppelklicken Sie darauf oder klicken Sie mit der rechten Maustaste und wählen **Barcode bearbeiten**. Beim Seriendruck erhält die Kopie jeder Person einen Barcode mit dem eigenen Wert dieser Person, als Bild in der Zeile gezeichnet, sodass er wie jedes Bild gedruckt und in PDF gespeichert wird. Der Wert ist nur Daten: Anführungszeichen oder ein Backslash darin ändern nichts. Ist ein Wert leer, gibt es keinen Barcode. Kann ein Code einen Wert nicht aufnehmen (ein Buchstabe in einem EAN-13 oder mehr als 500 Zeichen in Code 128), zeigt die Kopie an dieser Stelle eine kurze Meldung in eckigen Klammern, die anderen Kopien bleiben unverändert. Ein Seriendruck erstellt höchstens 2.000 Barcodes, wobei auch jeder Wert gezählt wird, der nicht dargestellt werden kann; wenn er meldet, dass es zu viele sind, drucken Sie weniger Personen. In einer gespeicherten .docx-Datei ist das Feld ein DISPLAYBARCODE-Feld mit der Spalte darin, sodass es beim erneuten Öffnen erhalten bleibt. Ein Feld mit einem eingegebenen Wert oder mit einem Typ, den JeffWrite nicht zeichnet (zum Beispiel UPC-E), bleibt unverändert. Seriendrucke per E-Mail fügen den Wert als Text ein.
- **Adressblock** fügt eine vollständige Postanschrift ein: Name, Firma, Straße und Ort. Wählen Sie die Zeilen aus, die Sie möchten. Eine Zeile ohne Inhalt wird weggelassen. Sie können auch ein Land angeben, das weggelassen werden soll, etwa Ihr eigenes. Die **Vorschau** zeigt die Adresse der ersten Person.
- **Grußzeile** fügt eine Zeile wie „Sehr geehrte Frau Dr. Fenwick,“ ein. Die Formulierung wählen Sie selbst. Die erste Auswahl, „Sehr geehrte Frau Dr. Fenwick,“, verwendet den Titel der Person. Hat eine Person keinen Titel, verwendet sie stattdessen Vor- und Nachnamen, etwa „Sehr geehrte Nora Pell,“. Hat eine Person keinen Namen, lautet die Grußzeile anders, etwa „Sehr geehrte Damen und Herren,“, und Sie können sie ändern.
- **Felder zuordnen** teilt dem Adressblock und der Grußzeile mit, welche Spalte welchen Teil enthält, wenn Ihre Spaltennamen ungewöhnlich sind.

## Regeln

Eine **Regel** macht eine Kopie für einige Personen anders. Klicken Sie auf **Regeln** und wählen Sie eine aus:

- **Wenn…Dann…Sonst**: Zeigt einen Text, wenn eine Spalte einen Wert hat, und einen anderen Text, wenn sie keinen hat. Wenn zum Beispiel das Bundesland „CA“ ist, schreiben Sie „Willkommen zu Hause“.
- **Datensatz überspringen falls**: Lässt eine Person aus, wenn eine Spalte eine Bedingung erfüllt, die Sie festlegen, etwa wenn der Nachname leer ist.
- **Nächster Datensatz** und **Nächsten Datensatz wenn**: Verwendet die Angaben der nächsten Person an derselben Stelle. Etiketten nutzen das, um auf jedes Etikett eine andere Person zu setzen.
- **Datensatz-Nr.** fügt die Nummer der Zeile der Person in der Liste ein. **Seriendrucknummer** zählt die bisher erstellten Kopien.
- **Abfragen** und **Ausfüllfeld**: Fragen Sie Sie beim Abschließen des Seriendrucks etwas. „Abfragen“ speichert die Antwort unter einem Namen. „Ausfüllen“ setzt die Antwort in den Text ein.
- **Textmarke setzen**: Gibt einem Namen einen Wert. Ein Querverweis auf den Namen zeigt den Wert an.

## Ergebnisse in der Vorschau prüfen

Klicken Sie auf **Sendungen > Ergebnisse Vorschau**. Es öffnet sich ein Bereich, der die Seite für eine Person zeigt. Verwenden Sie **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz**, um die Personen durchzugehen, oder geben Sie eine Nummer ein. **Empfänger suchen** springt zur nächsten Person, bei der eine Angabe Ihr Wort enthält.

**Seriendruckfelder hervorheben** hinterlegt die Seriendruckfelder auf Ihrer Seite grau, damit Sie sehen, wo sie stehen. Andere Felder, etwa Seitenzahlen, werden nicht hinterlegt. Die Hervorhebung erscheint nur auf dem Bildschirm und wird nie gedruckt.

Ihr eigenes Dokument wird dabei nicht verändert. **Auf Fehler prüfen** teilt Ihnen mit, wenn die Liste für ein Feld keine Spalte hat oder wenn niemand eingeschlossen ist.

## Seriendruck abschließen

Klicken Sie auf **Sendungen > Fertig stellen und zusammenführen**:

- **Einzelne Dokumente bearbeiten** erstellt ein neues Dokument mit der Kopie jeder Person. Sie können es vor dem Drucken noch ändern.
- **Dokumente drucken** erstellt dieses Dokument und öffnet die Seite **Drucken**.
- **Als PDF speichern** speichert alle Kopien in einer PDF-Datei.
- **E-Mail-Nachrichten senden** erstellt für jede Person eine .eml-Nachrichtendatei in einem Ordner, den Sie wählen. Wählen Sie die Spalte mit den E-Mail-Adressen und geben Sie einen Betreff ein. JeffWrite versendet die Nachrichten nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.

Personen, bei denen Sie das Häkchen in der Liste entfernt haben, werden ausgelassen.

Eine Nachrichtendatei wird nur für eine Person erstellt, deren Adresse genau eine E-Mail-Adresse enthält. Enthält eine Adresse zwei (mit Komma oder Semikolon dazwischen), steht ein Name davor oder fehlt die Domain, erhält diese Person keine Nachricht, und JeffWrite teilt Ihnen mit, wie viele es sind. Ein langer Betreff wird in Teilen gespeichert, die E-Mail-Programme wieder zusammensetzen.

Ein Seriendruck zeigt ein Fortschrittsfenster, und **Abbrechen** beendet ihn. Stellt eine Regel eine Frage (Abfragen oder Ausfüllfeld), werden Sie für jede unterschiedliche Frage einmal gefragt, bis zu 20 Fragen. Danach verwenden die übrigen Kopien ihre Standardantworten. Brechen Sie eine Frage ab, stoppt der gesamte Seriendruck. Ein Seriendruck stoppt und meldet dies, wenn er mehr als 10.000 Seiten erzeugen würde, damit ein sehr langes Dokument mit einer sehr langen Liste nicht stundenlang läuft. Führen Sie in diesem Fall einen Teil der Liste nach dem anderen zusammen.

## Umschläge

Klicken Sie auf **Sendungen > Umschläge**. Geben Sie die Empfängeradresse und Ihre Absenderadresse ein und wählen Sie die Größe des Umschlags aus der Liste. Die Größen sind nach Breite und Höhe benannt. Klicken Sie auf **Zum Dokument hinzufügen**, um den Umschlag auf einer eigenen Seite am Anfang Ihres Dokuments einzufügen, oder auf **Drucken**, um ihn zu drucken. Die Seite ist so breit wie die lange Seite des Umschlags, so wie die meisten Drucker Umschläge einziehen.

Um Umschläge für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Umschläge**. Jeder Umschlag enthält einen Adressblock.

## Etiketten

Klicken Sie auf **Sendungen > Etiketten**. Geben Sie die Adresse ein und wählen Sie den Bogen aus der Liste. Ein Bogen wird nach der Anzahl der Etiketten und der Größe jedes Etiketts benannt. Ist Ihr Bogen nicht in der Liste, wählen Sie **Eigener Bogen** und geben Sie die Maße ein: wie viele Etiketten nebeneinander und untereinander liegen, wie groß ein Etikett ist, der Abstand oben und seitlich vom ersten Etikett und wie weit die Etiketten voneinander entfernt sind.

- **Ganze Seite mit demselben Etikett** setzt auf jedes Etikett des Bogens dieselbe Adresse.
- **Einzelnes Etikett** setzt die Adresse auf ein einziges Etikett, damit Sie einen teilweise benutzten Bogen aufbrauchen können. Geben Sie an, welche Zeile und welche Spalte.

Klicken Sie auf **Neues Dokument**, um ein Dokument zu erstellen, das Sie ansehen und ändern können, oder auf **Drucken**, um es zu drucken.

Um Etiketten für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Etiketten**. Das erste Etikett enthält einen Adressblock, und jedes weitere Etikett beginnt mit einer Regel **Nächster Datensatz**. Wenn Sie das erste Etikett ändern, klicken Sie auf **Etiketten aktualisieren**, um die Änderung auf alle anderen zu übertragen.

## Was die Datei enthält

Die Art des Seriendrucks, der Name und der Speicherort der Liste, der Betreff und die E-Mail-Spalte werden in Ihrer .docx-Datei gespeichert. Welche Personen Sie abgewählt haben und Ihre Entscheidungen unter „Felder zuordnen“ werden nicht gespeichert. Sie werden nach dem Öffnen der Datei erneut abgefragt. Eine Liste wird bei jedem Öffnen erneut aus ihrer Datei gelesen. Befindet sich die Datei nicht mehr dort, sucht JeffWrite zunächst neben Ihrem Dokument und fragt Sie dann, wo die Datei liegt.

Eine Liste kann insgesamt bis zu 1.000 Spalten und 4.000.000 Zellen haben, und ein Seriendruck verwendet die ersten 100.000 Personen. Hat eine Datei mehr, verwendet JeffWrite, was hineinpasst, und teilt Ihnen das mit. Zeilen ohne jeglichen Inhalt werden aus der Liste ausgelassen.

## Siehe auch

- [Felder verwenden](fields)
- [Drucken](print)
- [Tabellen erstellen und bearbeiten](tables)
