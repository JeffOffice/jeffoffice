# Öffnen, Speichern, Drucken und Exportieren
<!-- keywords: Schriftarten in der Datei, eingebettete Schriftarten, fehlende Schriftart, E-Book, Ebook, EPUB, E-Reader, Kapitel, Inhalt, Webseite, HTML, als Webseite speichern, gefilterte Webseite, E-Mail, EML, Textmarken, PDF-Lesezeichen, Umschalt+F5, letzte Bearbeitung, wo Sie aufgehört haben, integrierte Vorlagen, Galerie, Brief, Anschreiben, Geschäftsbrief, Lebenslauf, Rechnung, Besprechungsagenda, Besprechungsnotizen, Flyer, Bericht, Kurzinformation, Manuskript, Newsletter, Öffnen, Speichern, Speichern unter, DOCX, DOC, DOT, ODT, OpenDocument, RTF, ältere Dateien, Vorlage, DOTX, zuletzt verwendet, anheften, neu, schließen, drucken, PDF, exportieren, wiederherstellen, automatische Wiederherstellung, nicht gespeichert, Absturz, Dateiansicht, Dateiformate, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, OTT, reparieren, Öffnen und reparieren, beschädigt, Kennwort, verschlüsseln, mit Kennwort schützen -->

Klicken Sie links oben auf **Datei**, um die Dateiansicht zu öffnen. Sie hat die Seiten Neu, Öffnen, Drucken und Exportieren sowie die Befehle Speichern und Speichern unter. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Ein neues Dokument beginnen

Drücken Sie **Strg+N**, oder klicken Sie auf **Datei > Neu** und dann auf **Leeres Dokument**. Ein neues Dokument öffnet sich in einem eigenen Fenster. Ist das Fenster, in dem Sie gerade arbeiten, leer und wurde nie gespeichert, ersetzt das neue Dokument es.

## Ein Dokument öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen**. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. JeffWrite öffnet diese Arten von Dateien:

- **.docx** ist ein Dokument.
- **.dotx** ist eine Vorlage. Eine Vorlage wird als neues Dokument geöffnet, das auf ihr basiert, sodass die Vorlage unverändert bleibt.
- **.docm** und **.dotm** sind ein Dokument und eine Vorlage, die **Makros** enthalten können, kleine Programme, die zu einer Datei gehören. JeffWrite führt sie nie aus. Beim Speichern als .docm oder .dotm bleiben sie erhalten. Wenn Sie als .docx oder .odt speichern, werden die Makros weggelassen, und JeffWrite weist Sie darauf hin.
- **.txt** ist Nur-Text, also nur die Wörter. Beim Öffnen liest JeffWrite die Datei sofort, wenn sie reines ASCII oder Unicode ist. Ist das nicht der Fall, fragt JeffWrite nach der **Codierung**, mit der die Datei gelesen werden soll, und zeigt die ersten Zeilen an, damit Sie prüfen können, ob die Buchstaben stimmen. Eine Codierung legt fest, wie eine Datei Buchstaben speichert, etwa UTF-8 oder Windows 1252. Beim Speichern als .txt wählen Sie die Codierung und die Zeilenenden. Die Datei enthält dann nur die Wörter: keine Schriftarten, Bilder oder Tabellenlinien.
- **.html** und **.htm** sind Webseiten. JeffWrite liest die Wörter, Überschriften, fette, kursive und unterstrichene Schrift, Farben, Listen und Tabellen. Aufzählungs- und Nummerierungszeichen werden als eingegebene Zeichen übernommen, Bilder und Links werden weggelassen. JeffWrite lädt nie etwas aus dem Internet und ignoriert Skripte. Eine Webseite wird nie überschrieben: **Speichern** fragt nach einem neuen Namen und speichert eine .docx-Datei. Zum Erstellen einer Webseite siehe unten **Als Webseite speichern**.
- **.doc** und **.dot** sind die älteren Dokument- und Vorlagenformate. JeffWrite liest sie und speichert Ihre Änderungen als neue .docx-Datei.
- **.odt**, **.ott** und **.fodt** sind OpenDocument-Dateien. JeffWrite liest alle drei Formate. Es speichert .odt-Dateien, und Ihre Änderungen an einer .ott- oder .fodt-Datei speichert es als neue .odt-Datei. Siehe [OpenDocument-Dateien öffnen und speichern](open-odt).
- **.rtf** ist Rich Text Format. JeffWrite liest und speichert es. Siehe [RTF-Dateien öffnen](open-rtf).

Die Seite **Öffnen** listet auch die Dokumente auf, die Sie zuletzt geöffnet oder gespeichert haben. Klicken Sie auf eines, um es zu öffnen. Mit der rechten Maustaste fixieren Sie ein Dokument oben in der Liste oder entfernen es aus der Liste.

Enthält eine Datei Teile, die JeffWrite noch nicht bearbeiten kann, etwa das Layout einer Tabelle oder eine Kopfzeile, bleiben sie erhalten. Siehe [Willkommen bei JeffWrite](start). Lässt sich ein Teil der Datei gar nicht lesen, nennt JeffWrite, was es weggelassen hat. Dann wird die Datei nicht überschrieben: **Speichern** fragt nach einem neuen Namen, damit das Original die Teile behält, die in der Kopie verloren gingen.

## Ein Dokument speichern

Drücken Sie **Strg+S**, um zu speichern. Beim ersten Mal fragt JeffWrite nach einem Namen. **Speichern unter** (F12) speichert eine Kopie unter einem neuen Namen. Wählen Sie **Vorlage (*.dotx)**, um eine Vorlage zu speichern, **Dokument mit Makros (*.docm)**, um Makros zu behalten, **Nur-Text (*.txt)** für die reinen Wörter oder **Rich Text Format (*.rtf)** für eine Datei, die andere Programme lesen können. Eine Datei, die aus einem älteren Format stammt, wird immer unter einem neuen Namen gespeichert, damit die alte Datei nie überschrieben wird.

## Eine Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**. Wählen Sie einen Namen. JeffWrite schreibt das Dokument in diese Datei, und Sie arbeiten weiter im Dokument, das Sie vorher hatten. Dessen Name und nicht gespeicherte Änderungen bleiben unverändert. Das ist praktisch, wenn Sie eine Fassung an jemanden senden und weiterschreiben möchten.

## Eine Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen** und schalten Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen** ein. Bei jedem Speichern kopiert JeffWrite zuerst die alte Datei in eine Datei namens „Sicherung von“ plus Dateiname, im selben Ordner. Lässt sich die Kopie nicht anlegen, fragt JeffWrite, ob ohne sie gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja sagen, und die Datei bleibt unverändert. So können Sie zur zuletzt gespeicherten Fassung zurückkehren.

## Als Vorlage speichern

Eine Vorlage ist ein Dokument, das Sie als Ausgangspunkt verwenden. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage landet unter **Meine Vorlagen**, und das Dokument, an dem Sie schreiben, bleibt unverändert. Im Dialog **Speichern unter** können Sie auch **Vorlage (*.dotx)** oder **OpenDocument-Textvorlage (*.ott)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um ein neues Dokument auf Basis Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen werden unter **Meine Vorlagen** aufgeführt. Klicken Sie auf eine. Ein neues Dokument mit dem Text und den Formatvorlagen der Vorlage öffnet sich. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Weitere Möglichkeiten, eine Datei zu öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken. Jede Möglichkeit fragt nach einer Datei oder verwendet die, auf die Sie geklickt haben:

- **Schreibgeschützt öffnen** öffnet die Datei nur zum Lesen. Sie können ansehen, blättern, suchen und kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** wählen, fragt JeffWrite nach einem neuen Namen, sodass die geöffnete Datei unverändert bleibt.
- **Als Kopie öffnen** öffnet ein neues Dokument mit dem Text der Datei. Es hat keine eigene Datei, daher fragt Speichern, wo es abgelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .docx- oder .doc-Datei, etwa eine, die beim Kopieren abgeschnitten wurde. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .docx- oder .doc-Datei beschädigt. Ein Kopiervorgang wurde auf halbem Weg abgebrochen, oder eine Festplatte hat Fehler gemacht. Eine .docx-Datei ist ein Bündel kleinerer Teile, etwa des Textes, der Formatvorlagen und der Bilder. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, das mitten im Vorgang abbrach, wird bis zur Stelle des Abbruchs übernommen. Ein Teil, das sich nicht lesen lässt, wird ausgelassen. Fehlen das Verzeichnis der Teile oder die Verknüpfungen zwischen ihnen, stellt JeffWrite sie wieder her.

Eine alte .doc-Datei ist ein großer Container mit einigen Datenströmen darin. Ein Strom enthält den Text, ein anderer die Tabelle, die festlegt, wo jeder Textabschnitt steht und wie er aussieht. Ist der Container abgeschnitten oder sind die Verzeichnisse beschädigt, sucht **Öffnen und reparieren** diese Ströme anhand ihres Inhalts und holt so viel wie möglich heraus. Sind die Listen, die festlegen, wie die Teile zusammengehören, beschädigt, nimmt das Programm jeden Teil als Fortsetzung des vorherigen und zeigt an, was es dort findet. Dieser Text könnte zu einer älteren Fassung des Dokuments oder zu einem anderen Teil der Datei gehören. Deshalb nennt die Meldung den Absatz, an dem er beginnt, und bittet Sie, ihn mit einer Sicherungskopie zu vergleichen, falls Sie eine haben. Liegt die Vermutung vor dem Text, meldet die Meldung, dass das ganze Dokument zusammengesetzt wurde. Liegt sie hinter dem Text, meldet sie, dass die Formatierung zusammengesetzt wurde. Konnten Bilder nicht vollständig wiederhergestellt werden, meldet die Meldung das ebenfalls. Ist die Tabelle verloren, bleibt der Text als einfache Absätze erhalten, ohne Formatvorlagen, Listen, Tabellen oder Bilder. Eine .doc-Datei mit Kennwort lässt sich nicht reparieren.

Das reparierte Dokument öffnet sich als Kopie mit einem Namen wie „Repariertes Brief“. Eine Meldung informiert Sie, was bis zu einem Abbruch erhalten blieb und was ausgelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Kopie unter einem eigenen Namen und prüfen Sie sie, bevor Sie ihr vertrauen.

## Nicht gespeicherte Arbeit wiederherstellen

JeffWrite speichert alle paar Minuten eine Reservekopie nicht gespeicherter Arbeit als Datei in einem eigenen Ordner. Wenn das Programm unerwartet beendet wird, bietet es diese Kopien beim nächsten Start an. Klicken Sie später auf **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen**, um sie anzusehen. Sie können auswählen, welche Kopien Sie öffnen, und jede an einem beliebigen Ort speichern.

## Mit einer Vorlage beginnen

**Datei > Neu** zeigt das **Leeres Dokument** und eine Reihe mitgelieferter Vorlagen, nach Art geordnet: Briefe (ein persönlicher Brief, ein Anschreiben und ein Geschäftsbrief), Berichte (ein Bericht, ein Bericht mit Deckblatt und Inhaltsverzeichnis sowie eine Kurzinformation), ein Manuskript, Publikationen (ein Newsletter und ein Flyer), Lebensläufe (ein klassischer und einer mit Seitenspalte) sowie Geschäftsunterlagen (eine Rechnung, eine Besprechungsagenda und Besprechungsnotizen). Zeigen Sie auf ein Bild, um zu sehen, wofür die Vorlage gedacht ist. Jedes Bild ist die erste Seite des Dokuments, das Sie erhalten. Eine **Vorlage** ist ein Dokument, das bereits eingerichtet ist, mit Formatvorlagen, Seitenrändern und Beispieltext. Klicken Sie eine an und ersetzen Sie den Beispieltext durch Ihren eigenen. Selbst gespeicherte Vorlagen finden Sie unter **Meine Vorlagen**.

## Dort weitermachen, wo Sie aufgehört haben

JeffWrite merkt sich, wo der Cursor stand, als Sie eine Datei gespeichert oder geschlossen haben. Beim nächsten Öffnen steht der Cursor dort. Drücken Sie **Umschalt+F5**, um zu den Stellen zu springen, an denen Sie zuletzt bearbeitet haben. Jeder Tastendruck springt zur davorliegenden Stelle: erst die letzten drei Stellen, dann zurück zur ersten.

## Als .odt speichern

Eine .odt-Datei kann nicht alles enthalten, was eine .docx-Datei enthalten kann. Beim Speichern als .odt listet JeffWrite zuerst auf, was verloren ginge, und lässt Sie stattdessen eine .docx-Datei speichern. Siehe [Info](info).

## Drucken

Drücken Sie **Strg+P**, oder klicken Sie auf **Datei > Drucken**. Wählen Sie einen Drucker, die Anzahl der Kopien und die Seiten, sehen Sie sich jede Seite in der Vorschau an und klicken Sie auf **Drucken**. Siehe [Drucken](print).

## Eine PDF-Datei erstellen

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Die PDF-Datei übernimmt Titel, Autor, Thema und Stichwörter aus **Datei > Info** (wenn **Dokumenteigenschaften einschließen** aktiviert ist), das Erstellungsdatum und JeffWrite als das Programm, mit dem sie erstellt wurde. Autor und ein aus dem Dateinamen abgeleiteter Titel bleiben weg, wenn persönliche Informationen entfernt werden. Jeder dieser Werte wird bis zu 4.000 Zeichen vollständig übernommen. Aktivieren Sie **PDF nach dem Speichern öffnen**, damit die Datei beim Speichern im PDF-Programm Ihres Computers geöffnet wird. Die Option ist zunächst deaktiviert. Lässt sich die Datei auf Ihrem Computer nicht öffnen, zeigt die Statusleiste das an; die Datei wird trotzdem gespeichert. Wählen Sie, welche Seiten aufgenommen werden: alle, die aktuelle Seite oder einen Bereich wie 1-3, 5 (auch p2s3 und s3 sind hier möglich). Wählen Sie dann, wie die Datei erstellt wird:

- **Optimieren für** hat **Standard**, das Ihre Bilder unverändert lässt, und **Minimale Größe**, das jedes Bild auf höchstens 150 Pixel pro Zoll reduziert, damit die Datei kleiner wird. Verwenden Sie „Minimale Größe“ für eine PDF-Datei, die Sie auf eine Webseite stellen oder per E-Mail senden.
- **Dokumenteigenschaften einschließen** übernimmt Titel, Autor, Thema und Stichwörter des Dokuments in die PDF-Datei.
- **Textmarken aus Überschriften erstellen** macht aus jeder Überschrift eine Textmarke, also einen Eintrag in der Liste, die ein PDF-Reader neben den Seiten anzeigt. Klicken Sie auf eine Textmarke, um zu ihrer Seite zu springen.
- **Dokumentstrukturtags für Barrierefreiheit** kennzeichnet Überschriften, Absätze, Listen, Tabellen (mit ihren Kopfzeilen) und Bilder (mit ihrem Alternativtext) und legt die Sprache fest, damit ein Screenreader die PDF-Datei in der richtigen Reihenfolge vorliest.

Wählen Sie dann einen Namen. Die PDF-Datei sieht auf jedem Computer gleich aus, weil die Schriftarten in ihr eingebettet sind. Die PDF-Datei, die **Drucken** erstellt, wenn Sie eine PDF-Datei wählen, enthält ebenfalls Textmarken und Tags. Werden auf einem Blatt mehrere Seiten angeordnet, führt jede Textmarke und jeder Link zu dem Blatt, auf dem seine Seite liegt. Die Seiten auf einem Blatt werden in der Reihenfolge gelesen, in der sie darauf liegen: von links nach rechts, dann von oben nach unten.

## Als Webseite speichern

Klicken Sie auf **Datei > Exportieren** und wählen Sie eine von zwei Arten von Webseiten. Eine **Webseite** ist eine Datei, die Sie in jedem Webbrowser öffnen können.

- **Als Webseite speichern** erstellt eine einzige .html-Datei mit Ihren Wörtern, deren Gestaltung und Ihren Bildern. Diese eine Datei können Sie allein versenden. Erhalten bleiben Schriftart, Schriftgrad, Farbe, Markierung, fett, kursiv, unterstrichen, Ausrichtung und Abstände um Absätze.
- **Als gefilterte Webseite speichern** erstellt eine kleine .html-Datei mit nur der Struktur: Überschriften, fette und kursive Schrift, Listen, Tabellen, Links und Bilder. Sie enthält keine Schriftarten, Schriftgrade oder Farben, sodass Sie ihr später eigene Gestaltung geben können.

Beide Varianten behalten Überschriften, Aufzählungs- und nummerierte Listen, Tabellen mit verbundenen Zellen, Links und Bilder bei. Ausgeblendeter Text, Wörter, die eine nachverfolgte Änderung gelöscht hat, Kommentare, Kopf- und Fußzeilen sowie Notizen werden weggelassen. Ein Link, der ein Programm starten oder ein Skript ausführen würde, wird als reiner Text gespeichert.

## Als E-Book speichern

Ein **E-Book** ist ein Buch, das Sie auf einem E-Reader, einem Telefon oder einem Tablet lesen. Die Datei ist ein **EPUB**. Der Text passt sich jedem Bildschirm an, sodass die Leserin oder der Leser die Schrift größer oder kleiner stellen kann.

1. Klicken Sie auf **Datei > Exportieren > Als E-Book (EPUB) speichern**.
2. Prüfen Sie **Titel** und **Autor**. Sie stammen zunächst aus den Eigenschaften Ihres Dokuments (**Datei > Info**). Für dieses Buch können Sie sie ändern.
3. Aktivieren Sie **Erstes Bild als Titelbild verwenden**, wenn das erste Bild in Ihrem Dokument das Titelbild sein soll. Ohne diese Option hat das Buch kein Titelbild.
4. Klicken Sie auf **OK** und wählen Sie dann den Speicherort.

So wird das Buch erstellt:

- Jede **Überschrift 1** beginnt ein neues Kapitel. Die Wörter vor der ersten Überschrift 1 bilden ein eigenes Kapitel.
- Das **Inhaltsverzeichnis** zeigt jede Überschrift 1, 2 und 3, und Leserinnen und Leser können zu jeder springen. Kleinere Überschriften stehen im Buch, aber nicht im Inhaltsverzeichnis.
- Bilder und Tabellen bleiben dort, wo Sie sie platziert haben. Der Alternativtext eines Bildes wird mit übernommen, damit ein Screenreader beschreiben kann, was es zeigt.
- Fußnoten und Endnoten stehen am Ende des Kapitels, in dem sie sich befinden. Jede Notiz enthält einen Link zurück zur Stelle im Text, und jede Nummer im Text ist ein Link zu ihrer Notiz.

Ein Buch unterliegt Grenzen, damit eine beschädigte Datei kein riesiges Buch erzeugen kann: bis zu 500 Kapitel, 4.000 Zeilen im Inhaltsverzeichnis, 1.000 Bilder (keines größer als 32 MB) und 20.000 Notizen. Was über eine Grenze hinausgeht, bleibt im letzten Kapitel oder wird zu reinem Text, und Ihre Wörter gehen nie verloren: Ein Bild über der Grenze wird durch seinen Alternativtext ersetzt, und eine Notiz über der Grenze erhält keine Gestaltung. Ein Bild, das ein Lesegerät nicht anzeigen kann (eine Metadatei, eine Bitmap oder ein TIFF), wird in ein PNG umgewandelt. Wird dasselbe gespeicherte Dokument erneut exportiert, entsteht dasselbe Buch, sodass Leserinnen und Leser ihre Stelle behalten.

## PDF-Datei mit Kennwort schützen

Klicken Sie im Feld **Als PDF exportieren** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer die PDF-Datei öffnet, muss dann das Kennwort eingeben. Das Kennwort schützt die Datei tatsächlich: Die Seiten werden mit starker Verschlüsselung (AES-256) verschlüsselt, und niemand kann sie ohne Kennwort lesen.

Sie können auch **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Reader beachten sie, aber sie hindern niemanden daran, der sie umgehen will. Verlassen Sie sich deshalb nicht darauf, eine Datei vertraulich zu halten.

Ein vergessenes Kennwort lässt sich nicht wiederherstellen. Bewahren Sie es daher an einem sicheren Ort auf. Das Kennwort gilt nur für diese eine PDF-Datei und wird nie gespeichert. Solange ein Kennwort vergeben ist, ist PDF/A im Feld deaktiviert, denn eine PDF/A-Datei kann kein Kennwort haben.

## PDF/A erstellen

Aktivieren Sie im Feld **Als PDF exportieren** die Option **PDF/A, zur langfristigen Aufbewahrung**. Eine PDF/A-Datei (PDF/A-1b) enthält jede verwendete Schriftart, benennt ihre Farben und folgt einem Standard für die Aufbewahrung von Dateien über viele Jahre. Sie kann für Screenreader gekennzeichnet werden, darf aber kein Kennwort haben.

## E-Mail erstellen

Klicken Sie auf **Datei > Exportieren > E-Mail erstellen**, um eine Nachrichtendatei (.eml) zu erstellen. Es wird nichts gesendet. Sie wählen, was in die Nachricht kommt: das Dokument, das Dokument als PDF oder die Seiten als Bilder in der Nachricht (die ersten 30 Seiten). Öffnen Sie die Datei in Ihrem E-Mail-Programm, tragen Sie die Empfängerin oder den Empfänger ein und senden Sie sie ab. Bei einer PDF-Datei können Sie im Feld auf **Mit Kennwort verschlüsseln…** klicken. Teilen Sie der Person das Kennwort auf anderem Weg mit, nicht in der Nachricht.

## Schriftarten in der Datei

Eine .docx-Datei kann die Schriftarten mitführen, in denen ihr Text gesetzt ist. So sieht das Dokument so aus, wie sein Autor es gestaltet hat, auch auf einem Computer, auf dem diese Schriftarten fehlen. JeffWrite liest sie beim Öffnen der Datei. Sie werden nur für dieses Dokument verwendet, solange es geöffnet ist. Sie werden nie auf Ihrem Computer installiert, kein anderes Dokument und kein anderes Programm sieht sie, und sie sind weg, sobald Sie das Dokument schließen. Eine Schriftart, die die Datei mitbringt, gilt nicht als fehlende Schriftart.

JeffWrite verwendet keine Schriftart, deren Lizenz das Einbetten untersagt, keine beschädigte Schriftart und keine Schriftart über den ersten 64 in einer Datei. Tritt das ein, sagt die Meldung beim Öffnen der Datei Bescheid, und der Text wird stattdessen in einer ähnlichen Schriftart angezeigt. Die Datei behält ihre Schriftarten so, wie sie beim Speichern waren.

Um die Schriftarten Ihres Textes in den gespeicherten Dateien mitzuspeichern, klicken Sie auf **Datei > Optionen**, öffnen die Seite **Speichern** und aktivieren **Schriftarten in der Datei speichern**. Die Option ist zunächst deaktiviert. JeffWrite nimmt beim Speichern einer .docx-Datei die Schriftarten auf, die mit dem Programm kommen und die der Text verwendet, in jedem Schnitt, den das Dokument nutzt. Eine Schriftart, deren Lizenz das nicht erlaubt, wird nie aufgenommen, und wird eine Schriftart weggelassen, nennt die Meldung nach dem Speichern ihren Namen. Die Datei wird dadurch größer, und in eine Datei passen höchstens 64 Schriftarten und 256 MB.

## Verwandte Themen

- [Willkommen bei JeffWrite](start)
- [Drucken](print)
- [JeffWrite mit der Tastatur bedienen](keyboard)
