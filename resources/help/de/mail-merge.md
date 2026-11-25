# Seriendruck und Katalogseriendruck
<!-- keywords: seriendruck, serienbrief, empfänger, empfängerliste, adressliste, etiketten, serienbriefe, csv, xlsx, vcard, seriendruckfeld, adressblock, katalog, produktliste, e-mail-seriendruck, vorschau -->

Der Seriendruck füllt eine Publikation mit Angaben aus einer Liste. Sie schreiben einen Brief nur einmal, und JeffPub erstellt für jede Person eine Kopie mit Name und Adresse dieser Person. Ein Katalogseriendruck wiederholt ein Layout für jeden Eintrag einer Liste.

Die Werkzeuge finden Sie auf der Registerkarte **Sendungen**. Für einen geführten Ablauf klicken Sie auf **Sendungen > Seriendruck starten > Seriendruck > Schritt-für-Schritt-Assistent für den Seriendruck**, um den Aufgabenbereich **Seriendruck** zu öffnen.

## Empfänger auswählen

Klicken Sie auf **Sendungen > Seriendruck starten > Empfänger auswählen**:

- **Vorhandene Liste verwenden…**: Wählen Sie eine CSV-Datei, eine Textdatei mit Tabstopp-Trennung, eine .xlsx-Tabellenkalkulation (das erste Blatt) oder eine .vcf-Kontaktdatei. Bei einer CSV-, Text- oder Tabellenkalkulationsdatei muss die erste Zeile die Spaltennamen enthalten.
- **Aus Kontakten auswählen…**: Wählen Sie eine Kontaktdatei (.vcf oder .vcard). Die meisten Adressbücher und Telefone können Kontakte als solche Datei speichern; jeder Kontakt wird zu einem Empfänger mit Spalten für Name, Adresse, Telefon und E-Mail.
- **Neue Liste eingeben…**: Füllen Sie das Fenster **Neue Adressliste** aus und verwenden Sie **Neuer Eintrag** für jede Person. JeffPub bietet dann an, die Liste als CSV-Datei zu speichern.

Um die Liste zu prüfen, klicken Sie auf **Sendungen > Seriendruck starten > Empfängerliste bearbeiten**. Im Fenster **Seriendruckempfänger** entfernen Sie das Häkchen bei einer Zeile, um diese Person auszulassen. Ein Klick auf eine Überschrift sortiert die Liste, und im Feld **Empfänger filtern** grenzen Sie die Liste ein. Mit **Empfänger unterscheiden anhand von** wählen Sie eine Spalte, die Personen voneinander unterscheidet. Die Liste wird mit ihren abgewählten Zeilen in Ihrer Publikation gespeichert. Wenn Sie dieselbe Listendatei erneut wählen, werden dieselben Personen wieder ausgelassen, auch wenn Sie die Zeilen vorher sortiert haben. Mit **Duplikate suchen** entfernen Sie bei Wiederholungen das Häkchen, und **Adressen überprüfen** hebt Zeilen mit fehlender Adresse oder einer ungewöhnlichen Postleitzahl hervor.

## Seriendruckfelder einfügen

Ein Seriendruckfeld ist ein Platzhalter, etwa «Vorname», der für jede Person durch einen echten Wert ersetzt wird. Klicken Sie in ein Textfeld (JeffPub fügt eines ein, wenn noch keines vorhanden ist) und verwenden Sie dann **Sendungen > Felder schreiben und einfügen**:

- **Adressblock**: Eine Postanschrift, die aus Spalten wie Vorname, Nachname und Adresszeile 1 gebildet wird.
- **Grußzeile**: „Sehr geehrte(r)“ mit dem Namen der Person oder „Liebe Freundin, lieber Freund,“, wenn kein Name vorhanden ist.
- **Seriendruckfeld einfügen**: Wählen Sie eine beliebige Spalte.
- **Bildfeld**: Wählen Sie eine Spalte mit Bilddateinamen. JeffPub fügt einen leeren Rahmen ein und sucht die Dateien neben Ihrer Liste. Nachdem Sie die Publikation gespeichert und erneut geöffnet haben, wird ein Bild, dessen Datei außerhalb des Ordners der Publikation liegt, erst angezeigt, wenn Sie die Empfängerliste erneut wählen (**Sendungen > Empfänger auswählen > Vorhandene Liste verwenden…**). Danach werden die Bilder im Ordner der Liste und in den darin enthaltenen Unterordnern angezeigt, bis Sie die Publikation schließen. JeffPub merkt sich dies nicht für das nächste Mal. Alternativ legen Sie die Bilddateien in den Ordner der Publikation oder in einen Unterordner davon; dann werden sie angezeigt, sobald die Publikation geöffnet wird. So liest eine Publikation, die Ihnen jemand schickt, nicht selbstständig Bilder von Ihrem Computer. Das Bild jedes Datensatzes wird angezeigt, wenn Sie die Ergebnisse in der Vorschau betrachten, drucken, eine PDF-Datei speichern oder in eine neue Publikation oder einen Katalog zusammenführen.

## Zahl, Preis, Datum oder Uhrzeit nach Wunsch anzeigen

Ein Seriendruckfeld zeigt den Wert genau so an, wie Ihre Liste ihn enthält. Um das zu ändern, klicken Sie neben das Feld (oder markieren Sie es bzw. mehrere Felder) und klicken Sie auf **Sendungen > Felder schreiben und einfügen > Seriendruckfeld formatieren…**. Wählen Sie unter **Wert anzeigen als**:

- **Zahl** mit 0 bis 4 **Dezimalstellen**. Zum Beispiel wird 1234,5 mit einer Dezimalstelle als 1.234,5 angezeigt.
- **Währung** mit dem Währungszeichen Ihrer Region: 1234,5 wird als 1.234,50 € angezeigt, wo dies das übliche Zeichen ist.
- **Prozent**, bei dem 0,256 mit einer Dezimalstelle als 25,6 % angezeigt wird.
- **Datum** mit einem **Format** wie Kurz, Lang, Jahr, Monat, Tag, Tag und Monat oder Monat und Jahr.
- **Uhrzeit** mit einem Format Kurz, 12-Stunden oder 24-Stunden.

Die Zeile **Beispiel** zeigt, wie Ihre Auswahl aussieht. Die Werte werden in der Sprache des Textes an der Feldstelle gelesen und angezeigt (bei fehlender Einstellung US-Englisch), nicht in der Sprache Ihres Computers. In US-Englisch ist 03/04/2026 der 4. März, in einer Sprache mit Tag vor Monat wie Deutsch oder britischem Englisch dagegen der 3. April; auf Deutsch ist 1.234,5 eine Zahl mit Dezimalkomma. Eine zweistellige Jahreszahl wird für 00 bis 29 als 2000 bis 2029 und für 30 bis 99 als 1930 bis 1999 gelesen. Eine Zahl in Klammern, etwa (500), ist negativ. Ein Wert, der keine Zahl, kein Datum und keine Uhrzeit der gewählten Art ist, wird genau so angezeigt, wie er in Ihrer Liste steht: ein Wort, „nan“ oder „inf“ (eine Person kann etwa Nan heißen), ein leerer Wert, eine Uhrzeit mit einem Datumsformat oder ein Datum mit einem Uhrzeitformat. Die Vorschau und der fertige Seriendruck zeigen beide den formatierten Wert. Die Auswahl ist ein Schritt für Rückgängig und wird mit der Publikation gespeichert.

## Ergebnisse in der Vorschau betrachten

Klicken Sie auf **Sendungen > Vorschau der Ergebnisse > Vorschau der Ergebnisse**, um echte Angaben anstelle der Felder zu sehen. Mit **Erster Datensatz**, **Vorheriger Datensatz**, **Nächster Datensatz** und **Letzter Datensatz** blättern Sie durch die Personen. Mit **Empfänger suchen** springen Sie zu einem Treffer.

## Seriendruck abschließen

Klicken Sie auf **Sendungen > Fertig stellen > Fertig stellen und zusammenführen**:

- **Auf Drucker zusammenführen…** öffnet die Seite **Drucken**. Aktivieren Sie **Alle Seriendruckdatensätze drucken** und drucken Sie dann.
- **In neuer Publikation zusammenführen** erstellt eine neue Publikation mit einer Kopie Ihrer Seiten für jede Person.
- **In PDF zusammenführen…** speichert eine PDF-Datei mit den Seiten aller Personen.
- **In E-Mail zusammenführen…** erstellt für jede Person eine .eml-E-Mail-Datei mit der ersten Seite als Bild. Wählen Sie die E-Mail-Spalte, geben Sie einen Betreff ein und wählen Sie einen Ordner. JeffPub versendet die E-Mails nicht. Öffnen Sie die Dateien in Ihrem E-Mail-Programm.
- **Jede als Datei speichern…** erstellt für jede Person eine .jpub-Publikation in einem Ordner Ihrer Wahl. Benennen Sie die Dateien mit Spalten in geschweiften Klammern, etwa {Nachname}, {Vorname}. Sie werden vor mehr als 200 Dateien gefragt, und einmal, wenn Dateien mit denselben Namen bereits vorhanden sind. Klicken Sie auf **Abbrechen**, um zwischen den Dateien anzuhalten.
- **Empfängerliste exportieren…** speichert die Liste als CSV-Datei.

## Katalogseriendruck

Klicken Sie auf **Einfügen > Seiten > Katalogseiten**, um den Aufgabenbereich **Katalogseriendruck** zu öffnen.

1. Wählen Sie die Produktliste, wie oben beschrieben.
2. Klicken Sie auf **Katalogbereich einfügen**. Es erscheint ein Kasten mit der Markierung „Katalogbereich“. Legen Sie seine **Zeilen** und **Spalten** sowie Position und Größe fest. Fügen Sie die Felder für einen Artikel in die erste Zelle ein, indem Sie sie in der Liste doppelt anklicken. Sie wiederholen sich in den übrigen Zellen. Für ein Bild wählen Sie die Version des Feldes mit „(Bild)“.
3. Prüfen Sie die Vorschau mit den Pfeilen. Jede Seite zeigt so viele Artikel, wie der Bereich Zellen hat. Führen Sie dann in eine neue Publikation, eine PDF-Datei oder auf den Drucker zusammen.

Mit **Katalogbereich entfernen** wird der Bereich entfernt.

## Siehe auch

- [Eine Publikation drucken](printing)
- [Textfelder hinzufügen und formatieren](text-boxes)
- [Die Registerkarte Sendungen](tab-mailings)
