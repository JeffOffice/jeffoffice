# Seriendruck und Katalogseriendruck
<!-- keywords: Seriendruck, Serienbrief, zusammenführen, Empfänger, Adressliste, Etiketten, Formbriefe, csv, xlsx, vcard, Seriendruckfeld, Adressblock, Katalog, Produktliste, E-Mail-Seriendruck, Vorschau -->

Beim Seriendruck wird eine Publikation mit Angaben aus einer Liste gefüllt. Sie schreiben einen Brief einmal, und JeffPub erstellt für jede Person eine Kopie mit deren Namen und Adresse an der richtigen Stelle. Ein Katalogseriendruck wiederholt ein Layout für jedes Element einer Liste.

Die Tools befinden sich auf der Registerkarte **Sendungen**. Wenn Sie Schritt für Schritt geführt werden möchten, klicken Sie auf **Sendungen > Seriendruck starten > Seriendruck > Seriendruck-Assistent mit Schritt-für-Schritt-Anweisungen**, um den Aufgabenbereich **Seriendruck** zu öffnen.

## Empfänger auswählen

Klicken Sie auf **Sendungen > Seriendruck starten > Empfänger auswählen**:

- **Vorhandene Liste verwenden…**: Wählen Sie eine CSV-Datei, eine durch Tabstopps getrennte Textdatei, eine .xlsx-Tabelle (das erste Tabellenblatt) oder eine .vcf-Kontaktdatei aus. In einer CSV-, Text- oder Tabellendatei muss die erste Zeile die Spaltennamen enthalten.
- **Aus Kontakten auswählen…**: Wählen Sie eine Kontaktdatei (.vcf oder .vcard) aus. Die meisten Adressbücher und Smartphones können ihre Kontakte in einer solchen Datei speichern. Jeder Kontakt wird zu einem Empfänger mit Spalten für Name, Adresse, Telefon und E-Mail.
- **Neue Liste eingeben…**: Füllen Sie das Fenster **Neue Adressliste** aus, und verwenden Sie für jede Person **Neuer Eintrag**. Anschließend bietet JeffPub an, die Liste als CSV-Datei zu speichern.

Um die Liste zu überprüfen, klicken Sie auf **Sendungen > Seriendruck starten > Empfängerliste bearbeiten**. Im Fenster **Seriendruckempfänger** deaktivieren Sie das Kontrollkästchen einer Zeile, um diese Person auszulassen, klicken auf eine Spaltenüberschrift, um zu sortieren, und geben Text in **Empfänger filtern** ein, um die Liste einzugrenzen. **Duplikate suchen** deaktiviert doppelte Einträge, und **Adressen überprüfen** hebt Zeilen mit fehlender Adresse oder ungewöhnlicher Postleitzahl hervor.

## Seriendruckfelder hinzufügen

Ein Seriendruckfeld ist ein Platzhalter, z. B. «Vorname», der für jede Person durch einen echten Wert ersetzt wird. Klicken Sie in ein Textfeld (andernfalls fügt JeffPub eines hinzu), und verwenden Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Adressblock**: eine Postanschrift, die aus Spalten mit Namen wie „First Name“, „Last Name“ und „Address Line 1“ zusammengesetzt wird.
- **Grußzeile**: „Liebe/r“ und der Name der Person oder „Liebe Freundin, lieber Freund,“, wenn kein Name vorhanden ist.
- **Seriendruckfeld einfügen**: Wählen Sie eine beliebige Spalte aus.
- **Bildfeld**: Wählen Sie eine Spalte mit Bilddateinamen aus. JeffPub fügt einen leeren Rahmen hinzu und sucht die Dateien im Ordner Ihrer Liste. Nachdem Sie die Publikation gespeichert und erneut geöffnet haben, wird ein Bild, dessen Datei außerhalb des Publikationsordners liegt, erst angezeigt, wenn Sie die Empfängerliste erneut auswählen (**Sendungen > Empfänger auswählen > Vorhandene Liste verwenden…**). Danach werden die Bilder im Ordner der Liste und in dessen Unterordnern angezeigt, bis Sie die Publikation schließen. JeffPub merkt sich dies nicht für das nächste Mal. Alternativ können Sie die Bilddateien im Publikationsordner oder in einem Unterordner davon ablegen; dann werden sie sofort beim Öffnen angezeigt. So wird verhindert, dass eine Publikation, die Ihnen jemand sendet, eigenmächtig Bilder von Ihrem Computer liest. Das Bild des jeweiligen Datensatzes wird angezeigt, wenn Sie eine Vorschau der Ergebnisse anzeigen, drucken, eine PDF-Datei speichern oder in einer neuen Publikation bzw. einem Katalog zusammenführen.

## Vorschau der Ergebnisse anzeigen

Klicken Sie auf **Sendungen > Vorschau Ergebnisse > Vorschau Ergebnisse**, um anstelle der Felder echte Angaben zu sehen. Mit **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz** blättern Sie durch die Personen, mit **Empfänger suchen** springen Sie zu einem Treffer.

## Seriendruck abschließen

Klicken Sie auf **Sendungen > Fertig stellen > Fertig stellen und zusammenführen**:

- **Auf Drucker zusammenführen…** öffnet die Seite **Drucken**. Aktivieren Sie **Alle Seriendruckdatensätze drucken**, und drucken Sie dann.
- **In neuer Publikation zusammenführen** erstellt eine neue Publikation mit einer Kopie Ihrer Seiten für jede Person.
- **In PDF zusammenführen…** speichert eine PDF-Datei mit den Seiten aller Personen.
- **In E-Mail zusammenführen…** erstellt pro Person eine .eml-E-Mail-Datei mit der ersten Seite als Bild. Wählen Sie die E-Mail-Spalte aus, geben Sie einen Betreff ein, und wählen Sie einen Ordner aus. JeffPub sendet die E-Mails nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.
- **Empfängerliste exportieren…** speichert die Liste als CSV-Datei.

## Katalogseriendruck

Klicken Sie auf **Einfügen > Seiten > Katalogseiten**, um den Aufgabenbereich **Katalogseriendruck** zu öffnen.

1. Wählen Sie die Produktliste wie oben beschrieben aus.
2. Klicken Sie auf **Katalogbereich einfügen**. Ein Feld mit der Bezeichnung „Katalogbereich“ wird angezeigt. Legen Sie die Werte für **Zeilen** und **Spalten** sowie Position und Größe fest. Fügen Sie die Felder für ein Element in die erste Zelle ein, indem Sie in der Liste darauf doppelklicken. Sie werden in den anderen Zellen wiederholt. Wählen Sie für ein Bild die Version „(Bild)“ eines Felds aus.
3. Zeigen Sie mit den Pfeilen eine Vorschau an. Jede Seite zeigt so viele Elemente, wie der Bereich Zellen hat. Führen Sie dann in einer neuen Publikation, einer PDF-Datei oder auf dem Drucker zusammen.

**Katalogbereich entfernen** entfernt den Bereich.

## Verwandte Themen

- [Publikation drucken](printing)
- [Textfelder hinzufügen und formatieren](text-boxes)
- [Die Registerkarte „Sendungen“](tab-mailings)
