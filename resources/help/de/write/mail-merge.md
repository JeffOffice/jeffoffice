# Seriendruck, Umschläge und Etiketten
<!-- keywords: seriendruck, zusammenführen, sendungen, empfänger, adressliste, serienbrief, etiketten, umschläge, seriendruckfeld, adressblock, grußzeile, regeln, wenn dann sonst, datensatz überspringen, nächster datensatz, vorschau ergebnisse, fertig stellen und zusammenführen, verzeichnis, e-mail-nachrichten, csv, xlsx, ods, vcard -->

Ein **Seriendruck** erstellt viele Kopien eines Briefes, wobei in jeder Kopie die Angaben einer anderen Person stehen. Sie schreiben den Brief nur einmal. Die Angaben der Personen bewahren Sie in einer **Liste** auf. JeffWrite setzt den Namen und die Adresse der ersten Person in die erste Kopie, die der zweiten Person in die zweite Kopie und so weiter.

Die Werkzeuge finden Sie auf der Registerkarte **Sendungen**. Für eine geführte Vorgehensweise klicken Sie auf **Sendungen > Seriendruck starten > Schritt-für-Schritt-Assistent für den Seriendruck**.

## Wählen Sie aus, was Sie erstellen

Klicken Sie auf **Sendungen > Seriendruck starten** und wählen Sie eine Option:

- **Briefe**: eine Kopie der Seite für jede Person. Jede Kopie beginnt auf einer neuen Seite.
- **E-Mail-Nachrichten**: eine Nachricht für jede Person. Siehe unten „E-Mail-Nachrichten senden“.
- **Umschläge** und **Etiketten**: eine Seite, die dafür erstellt wird. Siehe die Abschnitte unten.
- **Verzeichnis**: eine Liste aller Personen ohne Seitenumbruch zwischen den Personen. Verwenden Sie sie für eine Telefonliste oder einen Katalog.
- **Seriendrucktyp entfernen** macht das Dokument wieder zu einem gewöhnlichen Dokument.

JeffWrite speichert beim Speichern der Datei die Art des Seriendrucks und den Namen der Liste in der Datei. Wenn Sie die Datei erneut öffnen, ist der Seriendruck noch vorhanden.

## Wählen Sie Ihre Empfänger aus

Ein **Empfänger** ist eine Person, die eine Kopie erhält. Klicken Sie auf **Sendungen > Empfänger auswählen**:

- **Neue Liste eingeben**: Geben Sie die Personen in das Fenster ein, das sich öffnet. Klicken Sie für jede Person auf **Neuer Eintrag**. JeffWrite bietet dann an, die Liste als CSV-Datei zu speichern.
- **Vorhandene Liste verwenden**: Wählen Sie eine Datei aus, die bereits eine Liste ist. Das kann eine CSV-Datei oder eine Textdatei mit Tabstopps sein (eine Zeile für jede Person, und die erste Zeile enthält die Spaltennamen), eine Tabellenkalkulation (.xlsx, .xls oder .ods, aus JeffCrunch oder einem anderen Programm) oder eine Kontaktdatei (.vcf).
- **Aus Kontakten auswählen**: Wählen Sie eine Kontaktdatei (.vcf) aus. Die meisten Adressbücher und Telefone können ihre Kontakte als solche Datei speichern.

Klicken Sie dann auf **Sendungen > Empfängerliste bearbeiten**, um die Liste zu prüfen:

- Entfernen Sie den Haken bei einer Zeile, um diese Person auszulassen.
- Klicken Sie auf eine Spaltenüberschrift, um zu sortieren.
- Geben Sie in **Empfänger filtern** ein, um nur die Zeilen anzuzeigen, die Ihre Wörter enthalten.
- **Duplikate suchen** entfernt bei Wiederholungen den Haken.
- **Adressen überprüfen** markiert farbig die Zeilen, die keine Adresse oder eine merkwürdig aussehende Postleitzahl haben.

## Setzen Sie die Angaben in den Brief ein

Ein **Seriendruckfeld** ist ein Platzhalter. Es zeigt den Namen einer Spalte in doppelten spitzen Klammern, etwa «Vorname». In jeder Kopie wird es durch die Angabe dieser Person ersetzt.

Klicken Sie an die Stelle, an der die Angabe stehen soll, und verwenden Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Seriendruckfeld einfügen** listet die Spalten Ihrer Liste auf. Wählen Sie eine aus.
  - **Barcode aus Feld**, in derselben Liste, fügt einen Barcode ein, dessen Wert aus einer Spalte stammt, etwa eine Artikelnummer oder die 13-stellige ISBN eines Buches. Wählen Sie die Spalte, dann den Typ, und nehmen Sie die Einstellungen im Fenster **Barcode einfügen** vor (was Sie dort eingeben, ist nur ein Beispiel). Das Fenster bietet nur, was ein Feld tragen kann: den Typ, die Größe, die Balkenhöhe, die Ziffern und die QR-Stufe. Es lässt einen Buchpreis, einen Zusatzcode, eine Prüfziffer und den transparenten Hintergrund weg, da ein Feld dies nicht festlegen kann; die ISBN eines Buches ist ein EAN-13. Im Dokument zeigt das Feld den Spaltennamen in französischen Anführungszeichen. Beim Zusammenführen erhält die Kopie jeder Person einen Barcode mit dem eigenen Wert dieser Person, als Bild in die Zeile gezeichnet. Er wird also wie jedes Bild gedruckt und als PDF gespeichert. Der Wert ist nur Daten: Anführungszeichen oder ein Backslash darin ändern nichts. Ist ein Wert leer, entsteht kein Barcode. Kann ein Code einen Wert nicht aufnehmen (ein Buchstabe in einem EAN-13 oder mehr als 500 Zeichen in Code 128), zeigt die Kopie an dieser Stelle eine kurze Meldung in eckigen Klammern, und die anderen Kopien sind nicht betroffen. Ein Seriendruck erstellt höchstens 2.000 Barcodes, wobei jeder Wert mitgezählt wird, der nicht dargestellt werden kann. Führen Sie bei einer Meldung über zu viele Barcodes weniger Personen zusammen. In einer gespeicherten `.docx`-Datei ist das Feld ein DISPLAYBARCODE-Feld mit der Spalte darin, es bleibt also beim Speichern und erneuten Öffnen erhalten. Ein Feld mit einem eingegebenen Wert oder einem Typ, den JeffWrite nicht zeichnet (etwa UPC-E), bleibt unverändert. Bei E-Mail-Seriendrucken wird der Wert als Text eingefügt.
- **Adressblock** fügt eine vollständige Postanschrift ein: Name, Firma, Straße und Ort. Wählen Sie die Zeilen aus, die Sie möchten. Eine Zeile ohne Inhalt wird weggelassen. Sie können außerdem ein Land angeben, das weggelassen wird, etwa Ihr eigenes. Die **Vorschau** zeigt die Adresse der ersten Person.
- **Grußzeile** fügt eine Zeile wie „Sehr geehrte Frau Dr. Fenwick,“ ein. Die Wortwahl bestimmen Sie. Die erste Auswahl, „Sehr geehrte Frau Dr. Fenwick,“, verwendet die Anrede der Person. Hat eine Person keine Anrede, verwendet sie stattdessen Vor- und Nachnamen, etwa „Sehr geehrte Nora Pell,“. Hat eine Person keinen Namen, lautet die Grußzeile etwas anderes, etwa „Sehr geehrte Damen und Herren,“, was Sie ändern können.
- **Felder zuordnen** teilt dem Adressblock und der Grußzeile mit, welche Spalte welchen Teil enthält, wenn Ihre Spaltennamen ungewöhnlich sind.

## Regeln

Eine **Regel** macht eine Kopie für manche Personen anders. Klicken Sie auf **Regeln** und wählen Sie eine aus:

- **Wenn…Dann…Sonst**: Zeigt einen Text, wenn eine Spalte einen Wert hat, und einen anderen Text, wenn sie keinen hat. Wenn etwa im Feld Bundesland „CA“ steht, schreiben Sie „Willkommen zu Hause“.
- **Datensatz überspringen wenn**: Lässt eine Person aus, wenn eine Spalte einen Wert hat, etwa wenn der Nachname leer ist.
- **Nächster Datensatz** und **Nächsten Datensatz wenn**: Verwendet die Angaben der nächsten Person an derselben Stelle. Etiketten nutzen dies, um auf jedem Etikett eine andere Person einzusetzen.
- **Datensatz Nr.** fügt die Nummer der Zeile der Person in der Liste ein. **Seriendruck-Nr.** zählt die bisher erstellten Kopien.
- **Abfragen** und **Ausfüllen**: Fragt Sie beim Abschließen des Seriendrucks etwas. „Abfragen“ speichert die Antwort unter einem Namen. „Ausfüllen“ setzt die Antwort in den Text ein.
- **Textmarke setzen**: Gibt einem Namen einen Wert. Ein Verweis auf den Namen zeigt diesen Wert an.

## Vorschau der Ergebnisse

Klicken Sie auf **Sendungen > Vorschau Ergebnisse**. Es öffnet sich ein Bereich, der die Seite für eine Person zeigt. Verwenden Sie **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz**, um die Personen durchzugehen, oder geben Sie eine Nummer ein. **Empfänger suchen** springt zur nächsten Person, deren Angabe Ihre Wörter enthält.

**Seriendruckfelder hervorheben** unterlegt die Seriendruckfelder auf Ihrer Seite grau, damit Sie sehen, wo sie stehen. Andere Felder, etwa Seitenzahlen, werden nicht unterlegt. Die Unterlegung wird nur auf dem Bildschirm angezeigt und nie gedruckt.

Ihr eigenes Dokument wird nicht verändert. **Auf Fehler prüfen** meldet, wenn die Liste für ein Feld keine Spalte hat oder wenn keine Person aufgenommen wird.

## Seriendruck abschließen

Klicken Sie auf **Sendungen > Fertig stellen und zusammenführen**:

- **Einzelne Dokumente bearbeiten** erstellt ein neues Dokument mit der Kopie jeder Person. Sie können es vor dem Drucken ändern.
- **Dokumente drucken** erstellt dieses Dokument und öffnet die Seite **Drucken**.
- **Als PDF speichern** speichert alle Kopien in einer PDF-Datei.
- **E-Mail-Nachrichten senden** erstellt für jede Person eine Nachrichtendatei (.eml) in einem Ordner Ihrer Wahl. Wählen Sie die Spalte mit den E-Mail-Adressen aus und geben Sie einen Betreff ein. JeffWrite versendet die Nachrichten nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.

Personen, bei denen Sie den Haken in der Liste entfernt haben, werden ausgelassen.

Eine Nachrichtendatei wird nur für eine Person erstellt, deren Adresse genau eine E-Mail-Adresse enthält. Enthält eine Adresse zwei (mit Komma oder Semikolon dazwischen), steht ein Name davor oder fehlt die Domain, erhält diese Person keine Nachricht, und JeffWrite teilt Ihnen mit, wie viele Personen betroffen sind. Ein langer Betreff wird in Teilen gespeichert, die E-Mail-Programme wieder zusammensetzen.

Während des Seriendrucks zeigt ein Fortschrittsfenster den Stand an, mit **Abbrechen** halten Sie ihn an. Fragt eine Regel etwas (Abfragen oder Ausfüllen), werden Sie für jede verschiedene Frage einmal gefragt, höchstens 20 Fragen. Danach verwenden die übrigen Kopien ihre Standardantworten. Brechen Sie eine Frage ab, wird der gesamte Seriendruck angehalten. Ein Seriendruck stoppt und meldet dies, wenn er mehr als 10.000 Seiten erzeugen würde, damit ein sehr langes Dokument mit einer sehr langen Liste nicht stundenlang läuft. Führen Sie dann einen Teil der Liste nach dem anderen zusammen.

## Umschläge

Klicken Sie auf **Sendungen > Umschläge**. Geben Sie die Empfängeradresse und Ihre Absenderadresse ein und wählen Sie die Größe des Umschlags aus der Liste. Die Größen sind nach Breite und Höhe benannt. Klicken Sie auf **Zum Dokument hinzufügen**, um den Umschlag auf einer eigenen Seite am Anfang Ihres Dokuments einzufügen, oder auf **Drucken**, um ihn direkt zu drucken. Die Seite ist so breit wie die lange Seite des Umschlags, so nehmen die meisten Drucker Umschläge auf.

Um Umschläge für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Umschläge**. Jeder Umschlag enthält einen Adressblock.

## Etiketten

Klicken Sie auf **Sendungen > Etiketten**. Geben Sie die Adresse ein und wählen Sie den Bogen aus der Liste. Ein Bogen wird danach benannt, wie viele Etiketten er enthält und wie groß jedes Etikett ist. Wenn Ihr Bogen nicht in der Liste steht, wählen Sie **Eigener Bogen** und geben die Maße ein: wie viele Etiketten nebeneinander und untereinander stehen, wie groß ein Etikett ist, der Abstand oben und seitlich vom ersten Etikett sowie der Abstand zwischen den Etiketten.

- **Vollständige Seite mit demselben Etikett** setzt auf jedes Etikett des Bogens dieselbe Adresse.
- **Einzelnes Etikett** setzt die Adresse auf ein einziges Etikett, damit Sie einen teilweise benutzten Bogen aufbrauchen können. Geben Sie an, welche Zeile und welche Spalte.

Klicken Sie auf **Neues Dokument**, um ein Dokument zu erstellen, das Sie ansehen und ändern können, oder auf **Drucken**, um es direkt zu drucken.

Um Etiketten für einen Seriendruck zu erstellen, klicken Sie auf **Seriendruck starten > Etiketten**. Das erste Etikett enthält einen Adressblock, und jedes weitere Etikett beginnt mit einer Regel **Nächster Datensatz**. Wenn Sie das erste Etikett ändern, klicken Sie auf **Etiketten aktualisieren**, um die Änderung auf alle anderen zu übertragen.

## Was die Datei enthält

Die Art des Seriendrucks, der Name und der Ort der Liste, der Betreff und die E-Mail-Spalte werden in Ihrer .docx-Datei gespeichert. Welche Personen Sie abgewählt haben und Ihre Entscheidungen unter „Felder zuordnen“ werden nicht gespeichert. Nach dem erneuten Öffnen der Datei werden sie wieder abgefragt. Eine Liste wird bei jedem Mal aus ihrer Datei neu gelesen. Befindet sich die Datei nicht mehr am alten Ort, sucht JeffWrite neben Ihrem Dokument und fragt Sie dann, die Datei auszuwählen.

Eine Liste kann insgesamt bis zu 1.000 Spalten und 4.000.000 Zellen haben, und ein Seriendruck verwendet die ersten 100.000 Personen. Hat eine Datei mehr, verwendet JeffWrite, was hineinpasst, und teilt Ihnen dies mit. Zeilen ohne Inhalt werden aus der Liste ausgelassen.

## Siehe auch

- [Felder verwenden](fields)
- [Drucken](print)
- [Tabellen erstellen und ändern](tables)
