# Serienbriefe, Umschläge und Etiketten
<!-- keywords: Seriendruck, Serienbrief, Sendungen, Empfänger, Adressliste, Serienbrief erstellen, Etiketten, Umschläge, Seriendruckfeld, Adressblock, Grußzeile, Regeln, Wenn Dann Sonst, Datensatz überspringen, nächster Datensatz, Vorschau Ergebnisse, Fertig stellen und zusammenführen, Verzeichnis, E-Mail-Nachrichten, CSV, XLSX, ODS, vCard -->

Ein **Seriendruck** erstellt viele Kopien desselben Briefes, wobei in jeder Kopie die Angaben einer anderen Person stehen. Sie schreiben den Brief einmal. Die Angaben der Personen bewahren Sie in einer **Liste** auf. JeffWrite setzt den Namen und die Adresse der ersten Person in die erste Kopie, die der zweiten Person in die zweite Kopie und so weiter.

Die Werkzeuge finden Sie auf der Registerkarte **Sendungen**. Für eine geführte Schritt-für-Schritt-Anleitung klicken Sie auf **Sendungen > Seriendruck starten > Schritt-für-Schritt-Assistent für den Seriendruck**.

## Auswählen, was Sie erstellen

Klicken Sie auf **Sendungen > Seriendruck starten** und wählen Sie eine Option:

- **Briefe**: eine Kopie der Seite für jede Person. Jede Kopie beginnt auf einer neuen Seite.
- **E-Mail-Nachrichten**: eine Nachricht für jede Person. Siehe unten „E-Mail-Nachrichten senden“.
- **Umschläge** und **Etiketten**: eine Seite dafür. Siehe die Abschnitte unten.
- **Verzeichnis**: eine Liste aller Personen ohne Seitenumbruch zwischen den Personen. Verwenden Sie sie für eine Telefonliste oder einen Katalog.
- **Seriendrucktyp entfernen** macht das Dokument wieder zu einem gewöhnlichen Dokument.

JeffWrite speichert die Art des Seriendrucks und den Namen der Liste in der Datei, wenn Sie sie speichern. Öffnen Sie die Datei erneut, ist der Seriendruck noch vorhanden.

## Empfänger auswählen

Ein **Empfänger** ist eine Person, die eine Kopie erhält. Klicken Sie auf **Sendungen > Empfänger auswählen**:

- **Neue Liste eingeben**: Geben Sie die Personen in das Fenster ein, das sich öffnet. Klicken Sie für jede Person auf **Neuer Eintrag**. JeffWrite bietet danach an, die Liste als .csv-Datei zu speichern.
- **Vorhandene Liste verwenden**: Wählen Sie eine Datei, die bereits eine Liste ist. Das kann eine .csv-Datei oder eine Textdatei mit Tabstopps sein (eine Zeile für jede Person, und die erste Zeile enthält die Spaltennamen), eine Tabellenkalkulation (.xlsx, .xls oder .ods, aus JeffCrunch oder einem anderen Programm) oder eine Kontaktdatei (.vcf).
- **Aus Kontakten auswählen**: Wählen Sie eine Kontaktdatei (.vcf). Die meisten Adressbücher und Telefone können ihre Kontakte als solche Datei speichern.

Klicken Sie danach auf **Sendungen > Empfängerliste bearbeiten**, um die Liste zu prüfen:

- Entfernen Sie das Häkchen bei einer Zeile, um diese Person auszuschließen.
- Klicken Sie auf eine Spaltenüberschrift, um zu sortieren.
- Geben Sie unter **Empfänger filtern** ein Wort ein, um nur die Zeilen anzuzeigen, in denen Ihr Wort vorkommt.
- **Duplikate suchen** entfernt die Häkchen bei doppelten Einträgen.
- **Adressen überprüfen** markiert farbig die Zeilen, die keine Adresse oder eine fehlerhaft aussehende Postleitzahl haben.

## Die Angaben in den Brief einfügen

Ein **Seriendruckfeld** ist ein Platzhalter. Er zeigt den Namen einer Spalte in doppelten spitzen Anführungszeichen, etwa «Vorname». In jeder Kopie wird er durch die Angabe der jeweiligen Person ersetzt.

Klicken Sie an die Stelle, an die die Angabe soll, und verwenden Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Seriendruckfeld einfügen** listet die Spalten Ihrer Liste auf. Wählen Sie eine davon aus.
  - **Barcode aus Feld** steht in derselben Liste und fügt einen Barcode ein, dessen Wert eine Spalte ist, zum Beispiel eine Artikelnummer oder die 13-stellige ISBN eines Buchs. Wählen Sie die Spalte, dann Typ, und sehen Sie im Fenster **Barcode einfügen** nach (was Sie dort eintippen, ist nur ein Beispiel). Das Fenster bietet nur, was ein Feld enthalten kann: Typ, Größe, Balkenhöhe, Ziffern und QR-Stufe. Es lässt einen Buchpreis, einen Zusatzcode, eine Prüfziffer und den weißen Hintergrund weg, was ein Feld nicht angeben kann; die ISBN eines Buchs ist ein EAN-13. Im Dokument wird das Feld als Barcode dargestellt, bis zum Seriendruck als Beispiel des Typs. Um Typ oder Aussehen zu ändern, doppelklicken Sie darauf oder klicken es mit der rechten Maustaste an und wählen **Barcode bearbeiten**. Beim Seriendruck erhält die Kopie jeder Person einen Barcode mit dem eigenen Wert dieser Person, als Bild in die Zeile gezeichnet. Er wird daher wie jedes Bild gedruckt und in PDF gespeichert. Der Wert ist nur Daten: Anführungszeichen oder ein Backslash darin ändern nichts. Ist ein Wert leer, gibt es keinen Barcode. Kann ein Code einen Wert nicht aufnehmen (ein Buchstabe in einem EAN-13 oder mehr als 500 Zeichen in Code 128), zeigt die Kopie an dieser Stelle eine kurze Meldung in eckigen Klammern, die anderen Kopien bleiben unberührt. Ein Seriendruck erstellt höchstens 2.000 Barcodes, wobei auch jeder Wert gezählt wird, der nicht dargestellt werden kann; meldet das Programm zu viele, nehmen Sie weniger Personen in den Seriendruck auf. In einer gespeicherten .docx-Datei ist das Feld ein DISPLAYBARCODE-Feld mit der Spalte darin, es bleibt also beim Speichern und erneuten Öffnen erhalten. Ein Feld mit eingetragenem Wert oder einem Typ, den JeffWrite nicht zeichnet (zum Beispiel UPC-E), bleibt unverändert. E-Mail-Seriendrucke fügen den Wert als Text ein.
  - **Adressblock** fügt eine vollständige Postanschrift ein: Name, Firma, Straße und Ort. Wählen Sie die Zeilen aus, die Sie möchten. Eine Zeile ohne Inhalt wird weggelassen. Sie können außerdem ein Land angeben, das weggelassen werden soll, etwa Ihr eigenes. Mit **Vorschau** sehen Sie die Adresse der ersten Person.
  - **Grußzeile** fügt eine Zeile wie „Sehr geehrte Frau Dr. Fenwick,“ ein. Die Wortwahl bestimmen Sie. Die erste Auswahl, „Sehr geehrte Dr. Fenwick,“, verwendet die Anrede der Person. Hat eine Person keine Anrede, verwendet die Grußzeile stattdessen Vor- und Nachnamen, etwa „Sehr geehrte Nora Pell,“. Hat eine Person keinen Namen, lautet die Grußzeile anders, etwa „Sehr geehrte Damen und Herren,“, und Sie können sie ändern.
  - **Feldzuordnung** teilt dem Adressblock und der Grußzeile mit, welche Spalte welchen Teil enthält, wenn Ihre Spaltennamen ungewöhnlich sind.

## Regeln

Eine **Regel** macht eine Kopie für manche Personen anders. Klicken Sie auf **Regeln** und wählen Sie eine aus:

- **Wenn…Dann…Sonst**: Zeigt einen Text, wenn eine Spalte einen Wert hat, und einen anderen Text, wenn sie keinen hat. Ist zum Beispiel das Bundesland „CA“, schreiben Sie „Willkommen zu Hause“.
- **Datensatz überspringen wenn**: Lässt eine Person aus, wenn eine Spalte eine von Ihnen festgelegte Bedingung erfüllt, etwa wenn der Nachname leer ist.
- **Nächster Datensatz** und **Nächsten Datensatz wenn**: Verwendet die Angaben der nächsten Person an derselben Stelle. Etiketten nutzen dies, um auf jedem Etikett eine andere Person einzusetzen.
- **Datensatz Nr.** fügt die Nummer der Zeile der Person in der Liste ein. **Seriendrucknummer** zählt die bisher erstellten Kopien.
- **Abfragen** und **Ausfüllfeld**: Fragen Sie beim Abschluss des Seriendrucks nach einer Angabe. Abfragen speichert die Antwort unter einem Namen. Ausfüllfeld setzt die Antwort in den Text ein.
- **Textmarke setzen**: Gibt einem Namen einen Wert. Ein Querverweis auf den Namen zeigt den Wert.

## Ergebnisse in der Vorschau ansehen

Klicken Sie auf **Sendungen > Vorschau Ergebnisse**. Es öffnet sich ein Bereich mit der Seite für eine Person. Verwenden Sie **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz**, um durch die Personen zu blättern, oder geben Sie eine Nummer ein. Mit **Empfänger suchen** springen Sie zur nächsten Person, deren Angaben Ihr Wort enthalten.

**Seriendruckfelder hervorheben** unterlegt die Seriendruckfelder auf Ihrer Seite grau, damit Sie sehen, wo sie stehen. Andere Felder, etwa Seitenzahlen, werden nicht unterlegt. Die Hervorhebung erscheint nur auf dem Bildschirm und wird nie gedruckt.

Ihr eigenes Dokument wird dabei nicht verändert. **Auf Fehler prüfen** meldet, wenn die Liste für ein Feld keine Spalte hat oder wenn niemand eingeschlossen ist.

## Den Seriendruck abschließen

Klicken Sie auf **Sendungen > Fertig stellen und zusammenführen**:

- **Einzelne Dokumente bearbeiten** erstellt ein neues Dokument mit der Kopie jeder Person. Sie können es vor dem Drucken ändern.
- **Dokumente drucken** erstellt dieses Dokument und öffnet die Seite **Drucken**.
- **Als PDF speichern** speichert alle Kopien in einer PDF-Datei.
- **E-Mail-Nachrichten senden** erstellt für jede Person eine .eml-Nachrichtendatei in einem Ordner, den Sie wählen. Wählen Sie die Spalte mit den E-Mail-Adressen und geben Sie einen Betreff ein. JeffWrite versendet die Nachrichten nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.

Personen, bei denen Sie das Häkchen in der Liste entfernt haben, werden ausgelassen.

Eine Nachrichtendatei entsteht nur für eine Person, deren Adresse genau eine E-Mail-Adresse ist. Enthält eine Adresse zwei (mit Komma oder Semikolon dazwischen), hat sie einen Namen davor oder keine Domain, erhält diese Person keine Nachricht, und JeffWrite meldet, wie viele es sind. Ein langer Betreff wird in Teilen gespeichert, die E-Mail-Programme wieder zusammensetzen.

Während des Seriendrucks zeigt ein Fortschrittsfenster den Stand an, und **Abbrechen** beendet den Vorgang. Stellt eine Regel eine Frage (Abfragen oder Ausfüllfeld), werden Sie für jede verschiedene Frage einmal gefragt, bis zu 20 Fragen. Danach erhalten die übrigen Kopien ihre Standardantworten. Brechen Sie eine Frage ab, wird der gesamte Seriendruck beendet. Ein Seriendruck wird angehalten und dies gemeldet, wenn er mehr als 10.000 Seiten erzeugen würde, damit ein sehr langes Dokument mit einer sehr langen Liste nicht stundenlang läuft. Führen Sie in diesem Fall einen Teil der Liste nach dem anderen zusammen.

## Umschläge

Klicken Sie auf **Sendungen > Umschläge**. Geben Sie die Empfängeradresse und Ihre Absenderadresse ein und wählen Sie die Größe des Umschlags aus der Liste. Die Größen sind nach Breite und Höhe benannt. Klicken Sie auf **Zum Dokument hinzufügen**, um den Umschlag auf einer eigenen Seite am Anfang Ihres Dokuments einzufügen, oder auf **Drucken**, um ihn direkt zu drucken. Die Seite ist so breit wie die lange Seite des Umschlags, so wie die meisten Drucker Umschläge einziehen.

Um Umschläge für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Umschläge**. Jeder Umschlag enthält einen Adressblock.

## Etiketten

Klicken Sie auf **Sendungen > Etiketten**. Geben Sie die Adresse ein und wählen Sie den Bogen aus der Liste. Ein Bogen wird nach der Anzahl seiner Etiketten und der Größe jedes Etiketts benannt. Ist Ihr Bogen nicht in der Liste, wählen Sie **Eigener Bogen** und geben die Maße ein: wie viele Etiketten nebeneinander und untereinander liegen, wie groß ein Etikett ist, der Abstand oben und seitlich zum ersten Etikett sowie der Abstand zwischen den Etiketten.

- **Ganze Seite mit demselben Etikett** setzt auf jedes Etikett des Bogens dieselbe Adresse.
- **Einzelnes Etikett** setzt die Adresse auf ein einziges Etikett, damit Sie einen teilweise benutzten Bogen aufbrauchen können. Geben Sie die Zeile und die Spalte an.

Klicken Sie auf **Neues Dokument**, um ein Dokument zu erstellen, das Sie ansehen und ändern können, oder auf **Drucken**, um es direkt zu drucken.

Um Etiketten für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Etiketten**. Das erste Etikett enthält einen Adressblock, und jedes weitere Etikett beginnt mit einer Regel **Nächster Datensatz**. Ändern Sie das erste Etikett, klicken Sie auf **Etiketten aktualisieren**, um die Änderung auf alle anderen zu übertragen.

## Was die Datei enthält

Die Art des Seriendrucks, der Name und der Ort der Liste, der Betreff und die E-Mail-Spalte werden in Ihrer .docx-Datei gespeichert. Welche Personen Sie abgewählt haben und Ihre Auswahl bei der Feldzuordnung werden nicht gespeichert; danach werden sie erneut abgefragt. Eine Liste wird bei jeder Verwendung neu aus ihrer Datei gelesen. Befindet sich die Datei nicht mehr an ihrem Ort, sucht JeffWrite zuerst neben Ihrem Dokument und bittet Sie danach, sie auszuwählen.

Eine Liste kann insgesamt bis zu 1.000 Spalten und 4.000.000 Zellen haben, und ein Seriendruck verwendet die ersten 100.000 Personen. Hat eine Datei mehr, verwendet JeffWrite, was hineinpasst, und teilt Ihnen das mit. Zeilen ohne Inhalt werden aus der Liste ausgelassen.

## Siehe auch

- [Felder verwenden](fields)
- [Drucken](print)
- [Tabellen erstellen und ändern](tables)
