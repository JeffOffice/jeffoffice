# Seriendruck, Umschläge und Etiketten
<!-- keywords: seriendruck, serienbrief, mailings, empfänger, adressliste, serienbrief, etiketten, umschläge, seriendruckfeld, adressblock, grußzeile, regeln, wenn dann sonst, datensatz überspringen, nächster datensatz, vorschau der ergebnisse, fertig stellen und zusammenführen, verzeichnis, e-mail-nachrichten, csv, xlsx, ods, vcard -->

Ein **Seriendruck** erstellt viele Kopien eines Briefes, wobei in jeder Kopie die Angaben einer anderen Person stehen. Sie schreiben den Brief einmal. Die Angaben der Personen bewahren Sie in einer **Liste** auf. JeffWrite setzt den Namen und die Adresse der ersten Person in die erste Kopie, die der zweiten Person in die zweite Kopie und so weiter.

Die Werkzeuge finden Sie auf der Registerkarte **Sendungen**. Für eine geführte Vorgehensweise klicken Sie auf **Sendungen > Seriendruck starten > Schritt-für-Schritt-Assistent für den Seriendruck**.

## Festlegen, was Sie erstellen

Klicken Sie auf **Sendungen > Seriendruck starten** und wählen Sie eine Option:

- **Briefe**: eine Kopie der Seite für jede Person. Jede Kopie beginnt auf einer neuen Seite.
- **E-Mail-Nachrichten**: eine Nachricht für jede Person. Siehe „E-Mail-Nachrichten senden“ weiter unten.
- **Umschläge** und **Etiketten**: eine Seite, die dafür angelegt wird. Siehe die Abschnitte unten.
- **Verzeichnis**: eine Liste aller Personen ohne Seitenumbruch zwischen den Personen. Verwenden Sie sie für eine Telefonliste oder einen Katalog.
- **Seriendrucktyp entfernen** macht das Dokument wieder zu einem gewöhnlichen Dokument.

JeffWrite speichert die Art des Seriendrucks und den Namen der Liste in der Datei, wenn Sie sie speichern. Beim erneuten Öffnen der Datei ist der Seriendruck weiterhin vorhanden.

## Empfänger auswählen

Ein **Empfänger** ist eine Person, die eine Kopie erhält. Klicken Sie auf **Sendungen > Empfänger auswählen**:

- **Neue Liste eingeben**: Geben Sie die Personen in das Fenster ein, das sich öffnet. Klicken Sie für jede Person auf **Neuer Eintrag**. JeffWrite bietet danach an, die Liste als .csv-Datei zu speichern.
- **Vorhandene Liste verwenden**: Wählen Sie eine Datei aus, die bereits eine Liste ist. Das kann eine .csv-Datei oder eine Textdatei mit Tabstopps sein (eine Zeile für jede Person, und die erste Zeile enthält die Spaltenüberschriften), eine Tabellenkalkulation (.xlsx, .xls oder .ods aus JeffCrunch oder einem anderen Programm) oder eine Kontaktdatei (.vcf).
- **Aus Kontakten auswählen**: Wählen Sie eine Kontaktdatei (.vcf) aus. Die meisten Adressbücher und Telefone können ihre Kontakte als solche Datei speichern.

Klicken Sie danach auf **Sendungen > Empfängerliste bearbeiten**, um die Liste zu prüfen:

- Entfernen Sie den Haken bei einer Zeile, um diese Person auszulassen.
- Klicken Sie auf eine Spaltenüberschrift, um zu sortieren.
- Geben Sie in **Empfänger filtern** ein, um nur die Zeilen anzuzeigen, die Ihre Wörter enthalten.
- **Empfänger unterscheiden anhand von** wählt die Spalte aus, an der sich eine Person von einer anderen unterscheidet, etwa eine E-Mail-Adresse. Wenn Sie das Dokument erneut öffnen und die Liste gelesen wird, bleiben die Personen, bei denen Sie den Haken entfernt haben, wieder ausgelassen, auch wenn die Liste eine andere Reihenfolge hat oder neue Personen enthält. Mit **In der Liste platzieren** werden sie anhand ihrer Zeilennummer in der Listendatei gespeichert, auch wenn Sie die Zeilen in diesem Fenster sortiert oder gefiltert haben. Wenn die Liste sich so verändert hat, dass dies nicht mehr passt, teilt JeffWrite Ihnen das mit und nimmt alle Personen auf, die nicht zugeordnet werden konnten. Das Dokument speichert eine codierte Form jeder Adresse, nicht die Adresse selbst. Eine Person, die das Dokument besitzt und eine Adresse errät, kann diese Vermutung trotzdem überprüfen. **Dokument prüfen** (Datei > Info) entfernt die Angaben darüber, wer ausgelassen wurde.
- **Doppelte suchen** entfernt den Haken bei Wiederholungen.
- **Adressen überprüfen** färbt die Zeilen ein, denen eine Adresse fehlt oder deren Postleitzahl ungültig aussieht.

## Die Angaben in den Brief einfügen

Ein **Seriendruckfeld** ist ein Platzhalter. Es zeigt den Namen einer Spalte in doppelten spitzen Anführungszeichen, etwa «Vorname». In jeder Kopie wird es durch die Angabe der jeweiligen Person ersetzt.

Klicken Sie an die Stelle, an der die Angabe stehen soll, und verwenden Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Seriendruckfeld einfügen** listet die Spalten Ihrer Liste auf. Wählen Sie eine aus.
  - **Barcode aus Feld** in derselben Liste fügt einen Barcode ein, dessen Wert eine Spalte ist, etwa eine Artikelnummer oder die 13-stellige ISBN eines Buchs. Wählen Sie die Spalte, dann Typ und Aussehen im Fenster **Barcode einfügen** (was Sie dort eingeben, ist nur ein Beispiel). Das Fenster bietet nur, was ein Feld enthalten kann: den Typ, die Größe, die Balkenhöhe, die Ziffern und die QR-Stufe. Es lässt den Buchpreis, einen Zusatzcode, eine Prüfziffer und den hellen Hintergrund aus, die ein Feld nicht angeben kann; die ISBN eines Buchs ist ein EAN-13. Im Dokument wird das Feld bis zum Seriendruck als Barcode mit einem Beispiel des Typs dargestellt. Um Typ oder Aussehen zu ändern, doppelklicken Sie darauf oder klicken Sie mit der rechten Maustaste und wählen **Barcode bearbeiten**. Beim Zusammenführen erhält die Kopie jeder Person einen Barcode mit dem eigenen Wert dieser Person, als Bild in der Zeile gezeichnet. Er wird also wie jedes Bild gedruckt und in PDF gespeichert. Der Wert ist nur Daten: Anführungszeichen oder ein Backslash darin ändern nichts. Ist ein Wert leer, entsteht kein Barcode. Kann ein Code den Wert nicht aufnehmen (ein Buchstabe in einem EAN-13 oder mehr als 500 Zeichen in Code 128), zeigt die Kopie an dieser Stelle eine kurze Meldung in eckigen Klammern, und die anderen Kopien sind nicht betroffen. Ein Seriendruck erstellt höchstens 2.000 Barcodes, wobei jeder Wert zählt, der nicht dargestellt werden kann. Wenn die Meldung zu viele Barcodes anzeigt, führen Sie den Seriendruck für weniger Personen aus. In einer gespeicherten .docx-Datei ist das Feld ein DISPLAYBARCODE-Feld mit der Spalte darin, sodass es beim Speichern und erneuten Öffnen erhalten bleibt. Ein Feld mit einem eingetippten Wert oder einem Typ, den JeffWrite nicht zeichnet (zum Beispiel UPC-E), bleibt unverändert. Bei E-Mail-Seriendrucken wird der Wert als Text eingefügt.
- **Adressblock** fügt eine vollständige Postanschrift ein: Name, Firma, Straße und Ort. Wählen Sie die gewünschten Zeilen aus. Eine Zeile ohne Inhalt wird ausgelassen. Sie können außerdem ein Land angeben, das ausgelassen werden soll, etwa Ihr eigenes. Die **Vorschau** zeigt die Adresse der ersten Person.
- **Grußzeile** fügt eine Zeile ein wie „Sehr geehrte Frau Dr. Fenwick,“. Die Wörter wählen Sie selbst. Die erste Auswahl, „Sehr geehrte Frau Dr. Fenwick,“, verwendet den Titel der Person. Hat eine Person keinen Titel, verwendet sie stattdessen Vor- und Nachnamen, wie in „Sehr geehrte Nora Pell,“. Hat eine Person keinen Namen, sagt die Grußzeile etwas anderes, etwa „Sehr geehrte Damen und Herren,“, was Sie ändern können.
- **Felder zuordnen** teilt dem Adressblock und der Grußzeile mit, welche Spalte welchen Bestandteil enthält, wenn Ihre Spaltennamen ungewöhnlich sind.

## Regeln

Eine **Regel** macht eine Kopie für bestimmte Personen anders. Klicken Sie auf **Regeln** und wählen Sie eine aus:

- **Wenn…Dann…Sonst**: Zeigt einen Text, wenn eine Spalte einen Wert hat, und einen anderen Text, wenn sie keinen hat. Wenn zum Beispiel das Bundesland „CA“ ist, schreiben Sie „Willkommen zu Hause“.
- **Datensatz überspringen wenn**: Lässt eine Person aus, wenn eine Spalte eine von Ihnen festgelegte Bedingung erfüllt, etwa wenn der Nachname leer ist.
- **Nächster Datensatz** und **Nächsten Datensatz wenn**: Verwendet an derselben Stelle die Angaben der nächsten Person. Etiketten nutzen dies, um auf jedem Etikett eine andere Person einzusetzen.
- **Datensatz-Nr.** fügt die Nummer der Zeile der Person in der Liste ein. **Seriendrucknummer** zählt die bisher erstellten Kopien.
- **Abfragen** und **Ausfüllfeld**: Fragen Sie beim Abschluss des Seriendrucks nach einer Angabe. „Abfragen“ speichert die Antwort unter einem Namen. „Ausfüllfeld“ setzt die Antwort in den Text ein.
- **Textmarke setzen**: Gibt einem Namen einen Wert. Ein Querverweis auf den Namen zeigt den Wert an.

## Ergebnisse in der Vorschau prüfen

Klicken Sie auf **Sendungen > Vorschau der Ergebnisse**. Es öffnet sich ein Bereich, der die Seite für eine Person zeigt. Verwenden Sie **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz**, um durch die Personen zu blättern, oder geben Sie eine Zahl ein. **Empfänger suchen** springt zur nächsten Person, deren Angaben Ihre Wörter enthalten.

**Seriendruckfelder hervorheben** unterlegt die Seriendruckfelder auf Ihrer Seite grau, damit Sie sehen, wo sie stehen. Andere Felder, etwa Seitenzahlen, werden nicht unterlegt. Die Unterlegung ist nur auf dem Bildschirm sichtbar und wird nie gedruckt.

Ihr eigenes Dokument bleibt unverändert. **Fehler prüfen** teilt Ihnen mit, wenn die Liste für ein Feld keine Spalte hat oder wenn niemand aufgenommen wird.

## Seriendruck abschließen

Klicken Sie auf **Sendungen > Fertig stellen und zusammenführen**:

- **Einzelne Dokumente bearbeiten** erstellt ein neues Dokument mit der Kopie jeder Person. Sie können es vor dem Drucken ändern.
- **Dokumente drucken** erstellt dieses Dokument und öffnet die Seite **Drucken**.
- **Als PDF speichern** speichert alle Kopien in einer PDF-Datei.
- **E-Mail-Nachrichten senden** erstellt für jede Person eine .eml-Nachrichtendatei in einem Ordner Ihrer Wahl. Wählen Sie die Spalte mit den E-Mail-Adressen und geben Sie einen Betreff ein. JeffWrite versendet die Nachrichten nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.
- **Jede als Datei speichern** erstellt für jede Person eine Datei in einem Ordner Ihrer Wahl, als .docx- oder PDF-Datei. Benennen Sie die Dateien mit Spalten in geschweiften Klammern, etwa {Nachname}, {Vorname}; ein Name, der zweimal vorkommt, erhält eine Nummer. Ein Feld zeigt die Vornamen und wie viele Dateien erstellt werden. Sie werden vor mehr als 200 Dateien gefragt, und einmal, wenn im Ordner bereits Dateien mit denselben Namen liegen. Klicken Sie auf **Abbrechen**, um zwischen den Dateien anzuhalten: Die bereits erstellten Dateien bleiben erhalten; das Schließen des Fensters bewirkt dasselbe. Eine Datei, die eine Verknüpfung zu einer anderen Datei ist, wird durch eine neue Datei ersetzt; die Datei, auf die sie verwies, bleibt unberührt. Die Dateien enthalten nur den Brief für eine Person. Sie enthalten nicht die Liste, deren Speicherort oder die Angaben darüber, wer ausgelassen wurde. Ein Seriendruck erstellt höchstens 100.000 Dateien.

Die Personen, bei denen Sie in der Liste den Haken entfernt haben, werden ausgelassen.

Eine Nachrichtendatei entsteht nur für eine Person, deren Adresse genau eine E-Mail-Adresse enthält. Enthält eine Adresse zwei (mit Komma oder Semikolon dazwischen), steht ein Name davor oder fehlt die Domain, erhält diese Person keine Nachricht, und JeffWrite teilt Ihnen mit, wie viele es sind. Ein langer Betreff wird in Teilen gespeichert, die E-Mail-Programme wieder zusammensetzen.

Während eines Seriendrucks zeigt ein Fortschrittsfenster den Stand an; **Abbrechen** beendet ihn. Stellt eine Regel eine Frage (Abfragen oder Ausfüllfeld), wird Ihnen jede unterschiedliche Frage einmal gestellt, bis zu 20 Fragen. Danach verwenden die restlichen Angaben ihre Standardantworten. Brechen Sie eine Frage ab, stoppt der gesamte Seriendruck. Ein Seriendruck stoppt und meldet dies, wenn er mehr als 10.000 Seiten ergeben würde, damit ein sehr langes Dokument mit einer sehr langen Liste nicht stundenlang läuft. Führen Sie in diesem Fall den Seriendruck für einen Teil der Liste nach dem anderen aus.

## Umschläge

Klicken Sie auf **Sendungen > Umschläge**. Geben Sie die Empfängeradresse und Ihre Absenderadresse ein und wählen Sie die Größe des Umschlags aus der Liste. Die Größen sind nach Breite und Höhe benannt. Klicken Sie auf **Zum Dokument hinzufügen**, um den Umschlag am Anfang Ihres Dokuments auf eine eigene Seite zu setzen, oder auf **Drucken**, um ihn zu drucken. Die Seite ist so breit wie die lange Seite des Umschlags, so wie die meisten Drucker Umschläge einziehen.

Um Umschläge für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Umschläge**. Jeder Umschlag enthält einen Adressblock.

## Etiketten

Klicken Sie auf **Sendungen > Etiketten**. Geben Sie die Adresse ein und wählen Sie den Bogen aus der Liste. Ein Bogen wird nach der Anzahl seiner Etiketten und der Größe jedes Etiketts benannt. Ist Ihr Bogen nicht in der Liste, wählen Sie **Eigener Bogen** und geben die Maße ein: wie viele Etiketten nebeneinander und untereinander stehen, wie groß ein Etikett ist, der Abstand oben und seitlich des ersten Etiketts sowie wie weit die Etiketten voneinander entfernt sind.

- **Ganze Seite mit demselben Etikett** setzt auf jedes Etikett des Bogens dieselbe Adresse.
- **Einzelnes Etikett** setzt die Adresse auf ein einzelnes Etikett, damit Sie einen teilweise benutzten Bogen aufbrauchen können. Geben Sie an, welche Zeile und welche Spalte.

Klicken Sie auf **Neues Dokument**, um ein Dokument zu erstellen, das Sie ansehen und ändern können, oder auf **Drucken**, um es zu drucken.

Um Etiketten für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Etiketten**. Das erste Etikett enthält einen Adressblock, und jedes weitere Etikett beginnt mit einer Regel **Nächster Datensatz**. Wenn Sie das erste Etikett ändern, klicken Sie auf **Etiketten aktualisieren**, um die Änderung auf alle anderen zu übertragen.

## Was die Datei enthält

Die Art des Seriendrucks, Name und Speicherort der Liste, der Betreff und die Spalte für die E-Mail-Adressen werden in Ihrer .docx-Datei gespeichert. Ebenso wird gespeichert, welche Personen Sie ausgelassen haben, allerdings als kurze Codes der von Ihnen gewählten Spalte, nicht als Namen oder Adressen (nur eine .docx-Datei behält dies bei; Kompatibilität prüfen weist bei anderen Dateitypen darauf hin). Ihre Zuordnungen unter Felder zuordnen werden ebenfalls gespeichert. Eine Liste wird bei jedem Öffnen aus ihrer Datei neu gelesen. Liegt die Datei nicht mehr dort, wo sie war, sucht JeffWrite neben Ihrem Dokument und fragt Sie dann, die Datei auszuwählen.

Eine Liste kann insgesamt bis zu 1.000 Spalten und 4.000.000 Zellen haben. Ein Seriendruck verwendet die ersten 100.000 Personen. Enthält eine Datei mehr, verwendet JeffWrite, was hineinpasst, und teilt Ihnen das mit. Zeilen ohne Inhalt werden aus der Liste ausgelassen.

## Siehe auch

- [Felder verwenden](fields)
- [Drucken](print)
- [Tabellen erstellen und ändern](tables)
