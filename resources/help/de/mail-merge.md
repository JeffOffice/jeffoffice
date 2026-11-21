# Seriendruck und Katalogseriendruck
<!-- keywords: Seriendruck, zusammenführen, Empfänger, Adressliste, Etiketten, Serienbriefe, CSV, xlsx, vCard, Seriendruckfeld, Adressblock, Katalog, Produktliste, E-Mail-Seriendruck, Vorschau -->

Der Seriendruck füllt eine Publikation mit Angaben aus einer Liste. Sie schreiben einen Brief nur einmal, und JeffPub erstellt für jede Person eine Kopie mit dem jeweiligen Namen und der jeweiligen Adresse. Ein Katalogseriendruck wiederholt ein Layout für jeden Artikel in einer Liste.

Die Werkzeuge finden Sie auf der Registerkarte **Sendungen**. Für eine Schritt-für-Schritt-Anleitung klicken Sie auf **Sendungen > Seriendruck starten > Seriendruck > Schritt-für-Schritt-Assistent für den Seriendruck**, um den Aufgabenbereich **Seriendruck** zu öffnen.

## Empfänger auswählen

Klicken Sie auf **Sendungen > Seriendruck starten > Empfänger auswählen**:

- **Vorhandene Liste verwenden…**: Wählen Sie eine CSV-Datei, eine Textdatei mit Tabstopp-Trennung, eine .xlsx-Tabellenkalkulation (das erste Blatt) oder eine .vcf-Kontaktdatei. Bei einer CSV-, Text- oder Tabellenkalkulationsdatei muss die erste Zeile die Spaltennamen enthalten.
- **Aus Kontakten auswählen…**: Wählen Sie eine Kontaktdatei (.vcf oder .vcard). Die meisten Adressbücher und Telefone können ihre Kontakte als solche Datei speichern. Jeder Kontakt wird zu einem Empfänger, mit Spalten für Name, Adresse, Telefon und E-Mail.
- **Neue Liste eingeben…**: Füllen Sie das Fenster **Neue Adressliste** aus und verwenden Sie für jede Person **Neuer Eintrag**. Danach bietet JeffPub an, die Liste als CSV-Datei zu speichern.

Um die Liste zu prüfen, klicken Sie auf **Sendungen > Seriendruck starten > Empfängerliste bearbeiten**. Im Fenster **Seriendruckempfänger** entfernen Sie das Häkchen bei einer Zeile, um diese Person zu überspringen, klicken auf eine Überschrift, um zu sortieren, und geben unter **Empfänger filtern** ein, um die Liste einzugrenzen. Mit **Duplikate suchen** finden Sie doppelte Einträge, und **Adressen überprüfen** hebt Zeilen mit fehlender Adresse oder ungewöhnlicher Postleitzahl hervor.

## Seriendruckfelder einfügen

Ein Seriendruckfeld ist ein Platzhalter, etwa «Vorname», der für jede Person durch einen echten Wert ersetzt wird. Klicken Sie in ein Textfeld (fehlt es, fügt JeffPub eines hinzu) und verwenden Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Adressblock**: eine Postanschrift aus Spalten wie Vorname, Nachname und Adresszeile 1.
- **Grußzeile**: „Sehr geehrte“ und der Name der Person oder „Liebe Freundin, lieber Freund,“, wenn kein Name vorhanden ist.
- **Seriendruckfeld einfügen**: Wählen Sie eine beliebige Spalte.
- **Bildfeld**: Wählen Sie eine Spalte mit Bilddateinamen. JeffPub fügt einen leeren Rahmen ein und sucht die Dateien neben Ihrer Liste. Nachdem Sie die Publikation gespeichert und erneut geöffnet haben, wird ein Bild, dessen Datei außerhalb des Ordners der Publikation liegt, erst angezeigt, wenn Sie die Empfängerliste erneut auswählen (**Sendungen > Empfänger auswählen > Vorhandene Liste verwenden…**). Danach werden die Bilder im Ordner der Liste und in den Unterordnern angezeigt, bis Sie die Publikation schließen. JeffPub merkt sich das für das nächste Mal nicht. Alternativ legen Sie die Bilddateien in den Ordner der Publikation oder in einen Unterordner davon. Dann werden sie beim Öffnen sofort angezeigt. So liest eine Publikation, die Ihnen jemand schickt, keine Bilder von Ihrem Computer selbstständig ein. Das Bild jedes Datensatzes wird angezeigt, wenn Sie die Ergebnisse in der Vorschau betrachten, drucken, eine PDF-Datei speichern oder in eine neue Publikation oder einen Katalog zusammenführen.

## Zahl, Preis, Datum oder Uhrzeit anders anzeigen

Ein Seriendruckfeld zeigt den Wert so an, wie ihn Ihre Liste enthält. Um das zu ändern, klicken Sie neben das Feld (oder markieren Sie es bzw. mehrere Felder) und klicken auf **Sendungen > Felder schreiben und einfügen > Seriendruckfeld formatieren…**. Wählen Sie unter **Wert anzeigen als**:

- **Zahl** mit 0 bis 4 **Dezimalstellen**. Zum Beispiel wird 1234,5 mit 1 Dezimalstelle als 1.234,5 angezeigt.
- **Währung** mit dem Währungszeichen Ihrer Region: 1234,5 wird als 1.234,50 € angezeigt, sofern dies das passende Zeichen ist.
- **Prozent**, etwa 0,256 als 25,6 % mit 1 Dezimalstelle.
- **Datum** in einem **Format** wie Kurz, Lang, Jahr, Monat, Tag, Monat und Tag oder Monat und Jahr.
- **Uhrzeit** in einem Format Kurz, 12-Stunden oder 24-Stunden.

Die Zeile **Beispiel** zeigt, wie Ihre Auswahl aussieht. Die Werte werden in der Sprache des Textes an der Stelle des Feldes gelesen und angezeigt (Englisch (USA), wenn keine Sprache festgelegt ist), nicht in der Sprache Ihres Computers. Bei US-Englisch ist 03/04/2026 der 4. März, in einer Sprache mit Tag vor Monat wie Deutsch oder britischem Englisch dagegen der 3. April. Auf Deutsch ist 1.234,5 eine Zahl mit Dezimalkomma. Eine zweistellige Jahreszahl wird für 00 bis 29 als 2000 bis 2029 und für 30 bis 99 als 1930 bis 1999 gelesen. Eine Zahl in Klammern, etwa (500), ist negativ. Ein Wert, der keine Zahl, kein Datum und keine Uhrzeit der gewählten Art ist, wird genau so angezeigt, wie er in Ihrer Liste steht: ein Wort, „nan“ oder „inf“ (eine Person kann Nan heißen), ein leerer Wert, eine Uhrzeit mit einem Datumsformat oder ein Datum mit einem Uhrzeitformat. Die Vorschau und der fertige Seriendruck zeigen beide den formatierten Wert. Die Auswahl lässt sich mit einem Schritt rückgängig machen und wird mit der Publikation gespeichert.

## Ergebnisse der Vorschau anzeigen

Klicken Sie auf **Sendungen > Ergebnisse der Vorschau > Vorschau**, um echte Daten anstelle der Felder zu sehen. Mit **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz** blättern Sie durch die Personen, mit **Empfänger suchen** springen Sie zu einem Treffer.

## Seriendruck abschließen

Klicken Sie auf **Sendungen > Fertig stellen > Fertig stellen und zusammenführen**:

- **Auf Drucker zusammenführen…** öffnet die Seite **Drucken**. Aktivieren Sie **Alle Seriendruckdatensätze drucken** und drucken Sie dann.
- **In neuer Publikation zusammenführen** erstellt eine neue Publikation mit einer Kopie Ihrer Seiten für jede Person.
- **Als PDF zusammenführen…** speichert eine PDF-Datei mit den Seiten aller Personen.
- **In E-Mail zusammenführen…** erstellt für jede Person eine .eml-E-Mail-Datei mit der ersten Seite als Bild. Wählen Sie die E-Mail-Spalte, geben Sie einen Betreff ein und wählen Sie einen Ordner. JeffPub versendet die E-Mails nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.
- **Empfängerliste exportieren…** speichert die Liste als CSV-Datei.

## Katalogseriendruck

Klicken Sie auf **Einfügen > Seiten > Katalogseiten**, um den Aufgabenbereich **Katalogseriendruck** zu öffnen.

1. Wählen Sie die Produktliste, wie oben beschrieben.
2. Klicken Sie auf **Katalogbereich einfügen**. Es erscheint ein Feld mit der Markierung „Katalogbereich“. Legen Sie die **Zeilen** und **Spalten** sowie Position und Größe fest. Fügen Sie die Felder für einen Artikel in die erste Zelle ein, indem Sie sie in der Liste doppelklicken. Sie wiederholen sich in den anderen Zellen. Für ein Bild wählen Sie die Version des Feldes mit „(Bild)“.
3. Zeigen Sie mit den Pfeilen eine Vorschau an. Jede Seite zeigt so viele Artikel, wie der Bereich Zellen hat. Führen Sie danach in eine neue Publikation, in eine PDF-Datei oder auf den Drucker zusammen.

Mit **Katalogbereich entfernen** wird der Bereich entfernt.

## Siehe auch

- [Eine Publikation drucken](printing)
- [Textfelder hinzufügen und formatieren](text-boxes)
- [Die Registerkarte Sendungen](tab-mailings)
