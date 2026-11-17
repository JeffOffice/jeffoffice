# Öffnen, Speichern, Drucken und Exportieren
<!-- keywords: E-Book, Ebook, EPUB, E-Reader, Kapitel, Inhalt, Webseite, HTML, als Webseite speichern, gefilterte Webseite, E-Mail, eml, Textmarken, PDF-Textmarken, Umschalt+F5, letzte Bearbeitung, wo Sie aufgehört haben, integrierte Vorlagen, Galerie, Brief, Bericht, Kurzinformation, Manuskript, Newsletter, öffnen, speichern, speichern unter, docx, doc, dot, odt, OpenDocument, RTF, ältere Dateien, Vorlage, dotx, zuletzt verwendet, anheften, neu, schließen, drucken, PDF, exportieren, wiederherstellen, automatische Wiederherstellung, nicht gespeichert, Absturz, Dateiansicht, Dateiformate, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ott, reparieren, Öffnen und reparieren, beschädigt, Kennwort, verschlüsseln, mit Kennwort schützen -->

Klicken Sie oben links auf **Datei**, um die Dateiansicht zu öffnen. Sie hat die Seiten Neu, Öffnen, Drucken und Exportieren sowie die Befehle Speichern und Speichern unter. Mit **Esc** kehren Sie zu Ihrem Dokument zurück.

## Ein neues Dokument beginnen

Drücken Sie **Strg+N**, oder klicken Sie auf **Datei > Neu** und dann auf **Leeres Dokument**. Ein neues Dokument öffnet sich in einem eigenen Fenster. Wenn das aktuelle Fenster leer ist und noch nie gespeichert wurde, ersetzt das neue Dokument es.

## Ein Dokument öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen**. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. JeffWrite öffnet diese Dateiarten:

- **.docx** ist ein Dokument.
- **.dotx** ist eine Vorlage. Eine Vorlage öffnet sich als neues Dokument, das auf ihr basiert, sodass die Vorlage unverändert bleibt.
- **.docm** und **.dotm** sind ein Dokument und eine Vorlage, die **Makros** enthalten können, kleine Programme, die zu einer Datei gehören. JeffWrite führt sie nie aus. Beim Speichern als .docm oder .dotm bleiben sie erhalten. Wenn Sie als .docx oder .odt speichern, werden die Makros weggelassen, und JeffWrite weist Sie darauf hin.
- **.txt** ist reiner Text, nur die Wörter. Beim Öffnen liest JeffWrite die Datei sofort, wenn sie reines ASCII oder Unicode ist. Ist das nicht der Fall, fragt JeffWrite nach der **Codierung**, mit der die Datei gelesen werden soll, und zeigt die ersten Zeilen, damit Sie prüfen können, ob die Buchstaben stimmen. Eine Codierung legt fest, wie eine Datei Buchstaben speichert, zum Beispiel UTF-8 oder Windows 1252. Beim Speichern als .txt wählen Sie die Codierung und die Zeilenenden. Die Datei enthält dann nur die Wörter: keine Schriftarten, Bilder oder Tabellenlinien.
- **.html** und **.htm** sind Webseiten. JeffWrite liest die Wörter, Überschriften, Fett-, Kursiv- und Unterstreichungen, Farben, Listen und Tabellen. Aufzählungszeichen und Nummern von Listen werden als eingegebene Zeichen übernommen; Bilder und Links werden weggelassen. JeffWrite lädt nie etwas aus dem Internet und ignoriert Skripte. Eine Webseite wird nie überschrieben: **Speichern** fragt nach einem neuen Namen und speichert eine .docx-Datei. Zum Erstellen einer Webseite siehe unten **Als Webseite speichern**.
- **.doc** und **.dot** sind die älteren Dokument- und Vorlagenformate. JeffWrite liest sie, und Ihre Änderungen speichert es als neue .docx-Datei.
- **.odt**, **.ott** und **.fodt** sind OpenDocument-Dateien. JeffWrite liest alle drei. Es speichert .odt-Dateien, und Ihre Änderungen an einer .ott- oder .fodt-Datei speichert es als neue .odt-Datei. Siehe [OpenDocument-Dateien öffnen und speichern](open-odt).
- **.rtf** ist Rich Text Format. JeffWrite liest und speichert es. Siehe [RTF-Dateien öffnen](open-rtf).

Die Seite **Öffnen** listet auch die Dokumente auf, die Sie zuletzt geöffnet oder gespeichert haben. Klicken Sie auf eines, um es zu öffnen. Mit einem Klick der rechten Maustaste können Sie ein Dokument oben in der Liste anheften oder es aus der Liste entfernen.

Wenn eine Datei Teile enthält, die JeffWrite noch nicht bearbeiten kann, etwa das Layout einer Tabelle oder eine Kopfzeile, bleiben diese erhalten. Siehe [Willkommen bei JeffWrite](start). Kann ein Teil der Datei gar nicht gelesen werden, teilt JeffWrite mit, was es weggelassen hat. Dann wird die Datei auch nicht überschrieben: **Speichern** fragt nach einem neuen Namen, damit das Original das behält, was die Kopie verloren hat.

## Ein Dokument speichern

Drücken Sie **Strg+S**, um zu speichern. Beim ersten Mal fragt JeffWrite nach einem Namen. **Speichern unter** (F12) speichert eine Kopie unter einem neuen Namen. Wählen Sie **Vorlage (*.dotx)**, um eine Vorlage zu speichern, **Dokument mit Makros (*.docm)**, um Makros zu behalten, **Nur-Text (*.txt)** für die reinen Wörter oder **Rich Text Format (*.rtf)** für eine Datei, die andere Programme lesen können. Eine Datei, die aus einem älteren Dateiformat stammt, wird immer unter einem neuen Namen gespeichert, sodass die alte Datei nie überschrieben wird.

## Eine Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**. Wählen Sie einen Namen. JeffWrite schreibt das Dokument in diese Datei, und Sie arbeiten weiter an dem Dokument, das Sie vorher hatten. Dessen Name und die nicht gespeicherten Änderungen bleiben unverändert. Das ist praktisch, wenn Sie eine Fassung an jemanden schicken und weiterschreiben möchten.

## Eine Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen** und aktivieren Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffWrite zuerst die alte Datei in denselben Ordner, und zwar unter dem Namen „Sicherung von“ plus dem bisherigen Namen. Lässt sich die Kopie nicht anlegen, fragt JeffWrite, ob ohne Kopie gespeichert werden soll. Die Antwort lautet Nein, außer Sie sagen Ja, und die Datei bleibt dann unverändert. So können Sie zur zuletzt gespeicherten Fassung zurückkehren.

## Als Vorlage speichern

Eine Vorlage ist ein Dokument, mit dem Sie beginnen. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage landet unter **Meine Vorlagen**, und das Dokument, an dem Sie schreiben, bleibt unverändert. Unter **Speichern unter** können Sie auch **Vorlage (*.dotx)** oder **OpenDocument-Textvorlage (*.ott)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um ein neues Dokument aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen stehen unter **Meine Vorlagen**. Klicken Sie auf eine. Ein neues Dokument öffnet sich mit dem Text und den Formatvorlagen der Vorlage. Um eine Vorlage aus der Liste zu entfernen, klicken Sie mit der rechten Maustaste darauf und wählen **Aus Meine Vorlagen entfernen**.

## Weitere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken. Jede Möglichkeit fragt nach einer Datei oder verwendet die angeklickte:

- **Schreibgeschützt öffnen** öffnet die Datei nur zum Lesen. Sie können ansehen, blättern, suchen und kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie auf **Speichern** klicken, fragt JeffWrite nach einem neuen Namen, damit die geöffnete Datei unverändert bleibt.
- **Als Kopie öffnen** öffnet ein neues Dokument mit dem Text der Datei. Es hat noch keine eigene Datei, daher fragt **Speichern**, wohin es soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .docx- oder .doc-Datei, etwa eine, die beim Kopieren abgebrochen wurde. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .docx- oder .doc-Datei beschädigt. Ein Kopiervorgang ist auf halbem Weg abgebrochen, oder eine Festplatte hat Fehler gemacht. Eine .docx-Datei ist ein Bündel kleinerer Teile, etwa des Textes, der Formatvorlagen und der Bilder. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten abbricht, wird bis zu der Stelle übernommen, an der er abbricht. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verknüpfungen zwischen ihnen, stellt JeffWrite sie wieder her.

Eine alte .doc-Datei ist ein großer Container mit einigen Datenströmen darin. Ein Strom enthält den Text, ein anderer die Tabelle, die festlegt, wo welcher Textabschnitt steht und wie er aussieht. Ist der Container abgeschnitten oder sind seine Listen beschädigt, sucht **Öffnen und reparieren** diese Ströme anhand ihres Inhalts und liest so viel wie möglich. Geht die Tabelle verloren, bleibt der Text als einfache Absätze erhalten, ohne Formatvorlagen, Listen, Tabellen oder Bilder. Eine .doc-Datei mit Kennwort lässt sich nicht reparieren.

Das reparierte Dokument öffnet sich als Kopie mit einem Namen wie „Repariertes Brief“. Eine Meldung teilt Ihnen mit, was bis zu einer Unterbrechung erhalten blieb und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Kopie unter einem eigenen Namen, und prüfen Sie sie, bevor Sie ihr vertrauen.

## Nicht gespeicherte Arbeit wiederherstellen

JeffWrite speichert alle paar Minuten eine Reservekopie nicht gespeicherter Arbeit, als Datei in einem eigenen Ordner. Wird das Programm unerwartet beendet, bietet es diese Kopien beim nächsten Start an. Später sehen Sie sie über **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen**. Sie können auswählen, welche Kopie Sie öffnen, und jede dort speichern, wo Sie möchten.

## Mit einer Vorlage beginnen

**Datei > Neu** zeigt das **Leere Dokument** und eine Auswahl an Vorlagen, die mit JeffWrite geliefert werden, nach Art geordnet: Brief, Bericht, Kurzinformation, Manuskript und Newsletter. Jedes Bild zeigt die erste Seite des Dokuments, das Sie erhalten. Eine **Vorlage** ist ein bereits eingerichtetes Dokument mit Formatvorlagen, Seitenrändern und Beispieltext. Klicken Sie eine an und ersetzen Sie den Beispieltext durch Ihren eigenen. Selbst gespeicherte Vorlagen stehen unter **Meine Vorlagen**.

## Dort weitermachen, wo Sie aufgehört haben

JeffWrite merkt sich, wo der Cursor beim Speichern oder Schließen einer Datei stand. Beim nächsten Öffnen der Datei steht der Cursor an derselben Stelle. Mit **Umschalt+F5** springen Sie zu den zuletzt bearbeiteten Stellen. Jeder Tastendruck führt zur davor liegenden Stelle: zuerst zu den letzten drei Stellen, dann wieder zur ersten.

## Als .odt speichern

Eine .odt-Datei kann nicht alles enthalten, was eine .docx-Datei enthalten kann. Beim Speichern als .odt listet JeffWrite zuerst auf, was verloren gehen würde, und bietet an, stattdessen eine .docx-Datei zu speichern. Siehe [Info](info).

## Drucken

Drücken Sie **Strg+P**, oder klicken Sie auf **Datei > Drucken**. Wählen Sie einen Drucker, die Anzahl der Exemplare und die Seiten, sehen Sie sich jede Seite in der Vorschau an und klicken Sie auf **Drucken**. Siehe [Drucken](print).

## Ein PDF erstellen

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Wählen Sie die Seiten, die aufgenommen werden sollen: alle, die aktuelle Seite oder einen Bereich wie 1-3, 5 (p2s3 und s3 funktionieren hier ebenfalls). Legen Sie dann fest, wie es erstellt wird:

- **Optimieren für** bietet **Standard**, das Ihre Bilder unverändert lässt, und **Minimale Größe**, das jedes Bild auf höchstens 150 ppi reduziert, damit die Datei kleiner wird. Verwenden Sie die minimale Größe für ein PDF, das Sie auf eine Webseite stellen oder per E-Mail verschicken.
- **Dokumenteigenschaften einschließen** übernimmt Titel, Autor, Betreff und Stichwörter des Dokuments in das PDF.
- **Textmarken aus Überschriften erstellen** macht aus jeder Überschrift eine **Textmarke**, einen Eintrag in der Liste, die ein PDF-Reader neben den Seiten zeigt. Klicken Sie auf eine Textmarke, um zu deren Seite zu springen.
- **Dokumentstrukturtags für Barrierefreiheit** kennzeichnet Überschriften, Absätze, Listen, Tabellen (mit ihren Kopfzeilen) und Bilder (mit ihrem Alternativtext) und legt die Sprache fest, damit ein Vorleseprogramm das PDF in der richtigen Reihenfolge liest.

Wählen Sie dann einen Namen. Das PDF sieht auf jedem Computer gleich aus, weil die Schriftarten in der Datei enthalten sind. Auch das PDF, das **Drucken** erstellt, wenn Sie als Ziel eine PDF-Datei wählen, enthält Textmarken und Strukturtags. Wenn mehrere Seiten auf einem Blatt liegen, führt jede Textmarke und jeder Link zu dem Blatt, auf dem seine Seite liegt. Die Seiten auf einem Blatt werden in der Reihenfolge gelesen, in der sie darauf liegen: von links nach rechts, dann von oben nach unten.

## Als Webseite speichern

Klicken Sie auf **Datei > Exportieren** und wählen Sie eine von zwei Arten von Webseiten. Eine **Webseite** ist eine Datei, die Sie in jedem Webbrowser öffnen können.

- **Als Webseite speichern** erstellt eine einzige .html-Datei mit Ihren Wörtern, ihrem Aussehen und Ihren Bildern. Sie können diese eine Datei allein verschicken. Erhalten bleiben Schriftart, Schriftgrad, Farbe, Hervorhebung, Fett, Kursiv, Unterstreichung, Ausrichtung und der Abstand um Absätze.
- **Als gefilterte Webseite speichern** erstellt eine kleine .html-Datei, die nur die Struktur enthält: Überschriften, Fett und Kursiv, Listen, Tabellen, Links und Bilder. Schriftarten, Schriftgrade und Farben fehlen, sodass Sie später eigenes Aussehen festlegen können.

Beide Arten erhalten Überschriften, Aufzählungs- und nummerierte Listen, Tabellen mit verbundenen Zellen, Links und Bilder. Ausgeblendeter Text, Wörter, die eine nachverfolgte Änderung gelöscht hat, Kommentare, Kopf- und Fußzeilen sowie Notizen werden weggelassen. Ein Link, der ein Programm starten oder ein Skript ausführen würde, wird als einfacher Text gespeichert.

## Als E-Book speichern

Ein **E-Book** ist ein Buch, das Sie auf einem E-Reader, einem Telefon oder einem Tablet lesen. Die Datei ist ein **EPUB**. Der Text passt sich jeder Bildschirmgröße an, sodass die Leserin oder der Leser die Schrift größer oder kleiner stellen kann.

1. Klicken Sie auf **Datei > Exportieren > Als E-Book (EPUB) speichern**.
2. Prüfen Sie **Titel** und **Autor**. Sie übernehmen Titel und Autor aus den Eigenschaften Ihres Dokuments (**Datei > Info**). Sie können sie nur für dieses Buch ändern.
3. Aktivieren Sie **Erstes Bild als Titelbild verwenden**, wenn das erste Bild Ihres Dokuments das Titelbild sein soll. Ohne diese Option hat das Buch kein Titelbild.
4. Klicken Sie auf **OK** und wählen Sie dann den Speicherort der Datei.

So entsteht das Buch:

- Jede **Überschrift 1** beginnt ein neues Kapitel. Die Wörter vor der ersten Überschrift 1 bilden ein eigenes Kapitel.
- Das **Inhaltsverzeichnis** zeigt jede Überschrift 1, 2 und 3, und die Leserin oder der Leser kann direkt zu jeder springen. Kleinere Überschriften stehen im Buch, aber nicht im Inhaltsverzeichnis.
- Bilder und Tabellen bleiben an der Stelle, an die Sie sie gesetzt haben. Der Alternativtext eines Bildes wird mit ihm übernommen, damit ein Vorleseprogramm sagen kann, was es zeigt.
- Fußnoten und Endnoten stehen am Ende des Kapitels, in dem sie vorkommen. Jede Note enthält einen Link zurück zur Stelle im Text, und jede Nummer im Text ist ein Link zu ihrer Note.

Ein Buch unterliegt Grenzen, damit eine beschädigte Datei kein riesiges Buch erzeugt: höchstens 500 Kapitel, 4.000 Zeilen im Inhaltsverzeichnis, 1.000 Bilder (keines größer als 32 MB) und 20.000 Notizen. Was über eine Grenze hinausgeht, bleibt im letzten Kapitel oder wird zu einfachem Text, und Ihre Wörter gehen nie verloren: Ein Bild über der Grenze wird durch seinen Alternativtext ersetzt, und eine Notiz über der Grenze behält kein Aussehen. Ein Bild, das ein Leser nicht anzeigen kann (eine Metadatei, eine Bitmap oder eine TIFF-Datei), wird in ein PNG umgewandelt. Exportieren Sie dasselbe gespeicherte Dokument erneut, entsteht dasselbe Buch, sodass eine Leserin oder ein Leser die eigene Position behält.

## PDF mit Kennwort schützen

Klicken Sie im Feld **Als PDF exportieren** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer das PDF öffnet, muss dann das Kennwort eingeben. Das Kennwort schützt die Datei tatsächlich: Die Seiten werden mit starker Verschlüsselung (AES-256) unlesbar gemacht, und ohne Kennwort kann sie niemand lesen.

Sie können außerdem **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Reader halten sich daran, sie hindern aber niemanden, der sie umgehen möchte. Verlassen Sie sich daher nicht darauf, eine Datei damit privat zu halten.

Ein vergessenes Kennwort lässt sich nicht wiederherstellen; bewahren Sie es daher an einem sicheren Ort auf. Das Kennwort gilt nur für dieses eine PDF und wird nie gespeichert. Solange ein Kennwort festgelegt ist, ist **PDF/A** im Feld deaktiviert, da eine PDF/A-Datei kein Kennwort haben kann.

## PDF/A erstellen

Setzen Sie im Feld **Als PDF exportieren** ein Häkchen bei **PDF/A zur langfristigen Aufbewahrung**. Eine PDF/A-Datei (PDF/A-1b) enthält alle Schriftarten, die sie verwendet, benennt ihre Farben und folgt einer Norm für die Aufbewahrung über viele Jahre. Sie kann für Vorleseprogramme gekennzeichnet werden, aber kein Kennwort haben.

## E-Mail erstellen

Klicken Sie auf **Datei > Exportieren > E-Mail erstellen**, um eine Nachrichtendatei (.eml) zu erstellen. Es wird nichts gesendet. Sie wählen, was in die Nachricht kommt: das Dokument, das Dokument als PDF oder die angezeigten Seiten als Bilder in der Nachricht (die ersten 30 Seiten). Öffnen Sie die Datei in Ihrem E-Mail-Programm, tragen Sie die Empfängerin oder den Empfänger ein und senden Sie sie. Bei einem PDF können Sie im Feld auf **Mit Kennwort verschlüsseln…** klicken; teilen Sie das Kennwort dem Empfänger auf anderem Weg mit, nicht in der Nachricht.

## Siehe auch

- [Willkommen bei JeffWrite](start)
- [Drucken](print)
- [JeffWrite mit der Tastatur bedienen](keyboard)
