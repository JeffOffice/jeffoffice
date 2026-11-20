# Öffnen, Speichern, Drucken und Exportieren
<!-- keywords: Schriftarten in der Datei, eingebettete Schriftarten, fehlende Schriftart, E-Book, Ebook, EPUB, E-Reader, Kapitel, Inhalt, Inhaltsverzeichnis, Webseite, HTML, als Webseite speichern, gefilterte Webseite, E-Mail, eml, Textmarken, PDF-Lesezeichen, Umschalt+F5, letzte Bearbeitung, wo Sie aufgehört haben, integrierte Vorlagen, Katalog, Brief, Bericht, Kurzinformation, Manuskript, Newsletter, öffnen, speichern, speichern unter, docx, doc, dot, odt, OpenDocument, rtf, ältere Dateien, Vorlage, dotx, zuletzt verwendet, anheften, neu, schließen, drucken, PDF, exportieren, wiederherstellen, automatische Wiederherstellung, nicht gespeichert, Absturz, Dateibereich, Dateiformate, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ott, reparieren, Öffnen und reparieren, beschädigt, Kennwort, verschlüsseln, mit Kennwort schützen -->

Klicken Sie oben links auf **Datei**, um den Dateibereich zu öffnen. Er enthält die Seiten für **Neu**, **Öffnen**, **Drucken** und **Exportieren** sowie die Befehle **Speichern** und **Speichern unter**. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Ein neues Dokument beginnen

Drücken Sie **Strg+N**, oder klicken Sie auf **Datei > Neu** und dann auf **Leeres Dokument**. Ein neues Dokument öffnet sich in einem eigenen Fenster. Wenn das Fenster, in dem Sie gerade arbeiten, leer ist und nie gespeichert wurde, ersetzt das neue Dokument es.

## Ein Dokument öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen**. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. JeffWrite öffnet diese Arten von Dateien:

- **.docx** ist ein Dokument.
- **.dotx** ist eine Vorlage. Eine Vorlage öffnet sich als neues Dokument, das auf ihr basiert, sodass die Vorlage unverändert bleibt.
- **.docm** und **.dotm** sind ein Dokument und eine Vorlage, die **Makros** enthalten können, also kleine Programme, die zu einer Datei gehören. JeffWrite führt sie nie aus. Beim Speichern als .docm oder .dotm bleiben sie erhalten. Wenn Sie als .docx oder .odt speichern, werden die Makros weggelassen, und JeffWrite weist Sie darauf hin.
- **.txt** ist Nur-Text, nur die Wörter. Beim Öffnen liest JeffWrite die Datei sofort, wenn sie reines ASCII oder Unicode ist. Ist das nicht der Fall, fragt JeffWrite, mit welcher **Codierung** die Datei gelesen werden soll, und zeigt die ersten Zeilen an, damit Sie sehen können, ob die Buchstaben stimmen. Eine **Codierung** legt fest, wie eine Datei Buchstaben speichert, zum Beispiel UTF-8 oder Windows 1252. Beim Speichern als .txt wählen Sie die Codierung und die Zeilenenden. Die Datei enthält dann nur die Wörter: keine Schriftarten, Bilder oder Tabellenlinien.
- **.html** und **.htm** sind Webseiten. JeffWrite liest die Wörter, Überschriften, fett, kursiv, unterstrichen, Farben, Listen und Tabellen. Aufzählungszeichen und Nummern von Listen werden als eingegebene Zeichen übernommen; Bilder und Links werden weggelassen. JeffWrite lädt nie etwas aus dem Internet und ignoriert Skripte. Eine Webseite wird nie überschrieben: **Speichern** fragt nach einem neuen Namen und speichert eine .docx-Datei. Wie Sie eine Webseite erstellen, lesen Sie unten unter **Als Webseite speichern**.
- **.doc** und **.dot** sind die älteren Arten von Dokument und Vorlage. JeffWrite liest sie, und was Sie damit tun, speichert es als neue .docx-Datei.
- **.odt**, **.ott** und **.fodt** sind OpenDocument-Dateien. JeffWrite liest alle drei. Es speichert .odt-Dateien, und was Sie mit einer .ott- oder .fodt-Datei tun, speichert es als neue .odt-Datei. Siehe [OpenDocument-Dateien öffnen und speichern](open-odt).
- **.rtf** ist Rich Text Format. JeffWrite liest und speichert es. Siehe [RTF-Dateien öffnen](open-rtf).

Die Seite **Öffnen** listet außerdem die Dokumente auf, die Sie zuletzt geöffnet oder gespeichert haben. Klicken Sie auf eines, um es zu öffnen. Mit einem Rechtsklick heften Sie ein Dokument oben in der Liste an oder entfernen es aus der Liste.

Wenn eine Datei Teile enthält, die JeffWrite noch nicht bearbeiten kann, etwa das Layout einer Tabelle oder eine Kopfzeile, bleiben diese Teile erhalten. Siehe [Willkommen bei JeffWrite](start). Wenn ein Teil einer Datei gar nicht gelesen werden kann, teilt JeffWrite Ihnen mit, was es weggelassen hat. Dann wird diese Datei nicht überschrieben: **Speichern** fragt nach einem neuen Namen, damit das Original behält, was die Kopie verloren hat.

## Ein Dokument speichern

Drücken Sie **Strg+S**, um zu speichern. Beim ersten Mal fragt JeffWrite nach einem Namen. **Speichern unter** (F12) speichert eine Kopie unter einem neuen Namen. Wählen Sie **Vorlage (*.dotx)**, um eine Vorlage zu speichern, **Dokument mit Makros (*.docm)**, um Makros zu behalten, **Nur-Text (*.txt)** für nur die Wörter oder **Rich Text Format (*.rtf)** für eine Datei, die andere Programme lesen können. Eine Datei, die aus einer älteren Dateiart stammt, wird immer unter einem neuen Namen gespeichert, damit die alte Datei nie überschrieben wird.

## Eine Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**. Wählen Sie einen Namen. JeffWrite schreibt das Dokument in diese Datei, und Sie arbeiten weiter an dem Dokument, das Sie geöffnet hatten. Sein Name und seine nicht gespeicherten Änderungen bleiben unverändert. Das ist praktisch, wenn Sie eine Fassung an jemanden senden und weiterschreiben möchten.

## Eine Sicherungskopie behalten

Klicken Sie auf **Datei > Optionen** und schalten Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen** ein. Bei jedem Speichern kopiert JeffWrite zuerst die alte Datei in eine Datei mit dem Präfix „Sicherungskopie von“ und dem ursprünglichen Namen im selben Ordner. Lässt sich die Kopie nicht anlegen, fragt JeffWrite, ob ohne Kopie gespeichert werden soll. Die Antwort ist Nein, es sei denn, Sie sagen Ja, und die Datei bleibt dann unverändert. So können Sie zur letzten gespeicherten Fassung zurückkehren.

## Als Vorlage speichern

Eine Vorlage ist ein Dokument, das Sie als Ausgangspunkt verwenden. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage landet unter **Meine Vorlagen**, und das Dokument, an dem Sie schreiben, bleibt unverändert. Im Dialog **Speichern unter** können Sie auch **Vorlage (*.dotx)** oder **OpenDocument-Textvorlage (*.ott)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um ein neues Dokument aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen stehen unter **Meine Vorlagen**. Klicken Sie auf eine davon. Ein neues Dokument öffnet sich mit dem Text und den Formatvorlagen der Vorlage. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Weitere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Außerdem können Sie mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken. Jede Möglichkeit fragt nach einer Datei oder verwendet die Datei, auf die Sie geklickt haben:

- **Schreibgeschützt öffnen** öffnet die Datei nur zum Lesen. Sie können sie ansehen, blättern, suchen und kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** drücken, fragt JeffWrite nach einem neuen Namen, sodass die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet ein neues Dokument mit dem Text der Datei. Es hat keine eigene Datei, daher fragt **Speichern**, wo es abgelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .docx- oder .doc-Datei, etwa eine, die beim Kopieren abgebrochen wurde. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .docx- oder .doc-Datei beschädigt. Ein Kopiervorgang wurde auf halbem Weg abgebrochen, oder eine Festplatte hat Fehler gemacht. Eine .docx-Datei ist ein Bündel kleinerer Teile, etwa des Textes, der Formatvorlagen und der Bilder. **Öffnen und reparieren** liest jeden Teil, den es lesen kann. Ein Teil, der mitten im Vorgang abbricht, wird bis zu der Stelle übernommen, an der er endet. Ein Teil, das nicht gelesen werden kann, wird weggelassen. Fehlen die Liste der Teile oder die Verknüpfungen zwischen ihnen, stellt JeffWrite sie wieder her.

Eine alte .doc-Datei ist ein großer Container mit einigen Datenströmen darin. Ein Strom enthält den Text, ein anderer die Tabelle, die festlegt, wo jedes Textstück steht und wie es aussieht. Ist der Container abgeschnitten oder sind seine Listen beschädigt, sucht **Öffnen und reparieren** diese Ströme anhand ihres Inhalts und liest so viel wie möglich. Geht die Tabelle verloren, bleibt der Text als einfache Absätze erhalten, ohne Formatvorlagen, Listen, Tabellen oder Bilder. Eine .doc-Datei mit Kennwort kann nicht repariert werden.

Das reparierte Dokument öffnet sich als Kopie mit einem Namen wie „Reparierter Brief“. Eine Meldung teilt Ihnen mit, was bis zu einem Bruch erhalten blieb und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Kopie unter einem eigenen Namen und prüfen Sie sie, bevor Sie ihr vertrauen.

## Nicht gespeicherte Arbeit wiederherstellen

JeffWrite speichert alle paar Minuten eine Reservekopie nicht gespeicherter Arbeit als Datei in einem eigenen Ordner. Wird das Programm unerwartet beendet, bietet es diese Kopien beim nächsten Start an. Sie können sie auch später über **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen** ansehen. Sie wählen aus, welche Kopien Sie öffnen, und können jede an einem beliebigen Ort speichern.

## Mit einer Vorlage beginnen

**Datei > Neu** zeigt das **Leere Dokument** und eine Auswahl an Vorlagen, die JeffWrite mitbringt, nach Art gruppiert: Brief, Bericht, Kurzinformation, Manuskript und Newsletter. Jedes Bild zeigt die erste Seite des Dokuments, das Sie erhalten. Eine **Vorlage** ist ein bereits eingerichtetes Dokument mit Formatvorlagen, Seitenrändern und Beispieltext. Klicken Sie eine an und ersetzen Sie den Beispieltext durch Ihren eigenen. Selbst gespeicherte Vorlagen stehen unter **Meine Vorlagen**.

## Dort weitermachen, wo Sie aufgehört haben

JeffWrite merkt sich, wo sich die Einfügemarke befand, als Sie eine Datei gespeichert oder geschlossen haben. Beim nächsten Öffnen der Datei steht die Einfügemarke wieder dort. Drücken Sie **Umschalt+F5**, um zu den Stellen zu springen, an denen Sie zuletzt bearbeitet haben. Jeder Tastendruck führt zur jeweils davorliegenden Stelle: zuerst die letzten drei Stellen, dann wieder zur ersten.

## Als .odt speichern

Eine .odt-Datei kann nicht alles enthalten, was eine .docx-Datei enthalten kann. Beim Speichern als .odt zeigt JeffWrite zuerst an, was verloren ginge, und bietet an, stattdessen eine .docx-Datei zu speichern. Siehe [Info](info).

## Drucken

Drücken Sie **Strg+P**, oder klicken Sie auf **Datei > Drucken**. Wählen Sie einen Drucker, die Anzahl der Exemplare und die Seiten, prüfen Sie jede Seite in der Vorschau und klicken Sie auf **Drucken**. Siehe [Drucken](print).

## Eine PDF-Datei erstellen

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Die PDF-Datei übernimmt Titel, Autor, Betreff und Stichwörter aus **Datei > Info** (wenn **Dokumenteigenschaften einschließen** aktiviert ist), außerdem das Erstellungsdatum und JeffWrite als erzeugendes Programm. Der Autor und ein aus dem Dateinamen gebildeter Titel bleiben weg, wenn persönliche Informationen entfernt werden. Jeder dieser Werte wird bis zu 4.000 Zeichen vollständig übernommen. Aktivieren Sie **PDF nach dem Speichern öffnen**, damit die Datei beim Speichern im PDF-Programm Ihres Computers geöffnet wird. Die Option ist zunächst ausgeschaltet. Lässt sich die Datei auf Ihrem Computer nicht öffnen, zeigt die Statusleiste das an; die Datei ist dann trotzdem gespeichert. Wählen Sie, welche Seiten aufgenommen werden: alle, die aktuelle Seite oder einen Bereich wie 1-3, 5 (p2s3 und s3 funktionieren hier ebenfalls). Legen Sie danach fest, wie die Datei erstellt wird:

- **Optimieren für** bietet **Standard**, das Bilder unverändert lässt, und **Minimale Größe**, das Bilder auf höchstens 150 ppi reduziert, damit die Datei kleiner wird. Verwenden Sie „Minimale Größe“ für ein PDF, das Sie auf einer Webseite veröffentlichen oder per E-Mail versenden.
- **Dokumenteigenschaften einschließen** übernimmt Titel, Autor, Betreff und Stichwörter des Dokuments in die PDF-Datei.
- **Textmarken aus Überschriften erstellen** macht aus jeder Überschrift eine **Textmarke**, also einen Eintrag in der Liste, die ein PDF-Reader neben den Seiten anzeigt. Klicken Sie auf eine Textmarke, um zu ihrer Seite zu springen.
- **Strukturtags für Barrierefreiheit** kennzeichnet Überschriften, Absätze, Listen, Tabellen (mit ihren Kopfzeilen) und Bilder (mit ihrem Alternativtext) und legt die Sprache fest, damit ein Screenreader das PDF in der richtigen Reihenfolge vorliest.

Wählen Sie dann einen Namen. Das PDF sieht auf jedem Computer gleich aus, weil die Schriftarten in der Datei enthalten sind. Das PDF, das **Drucken** erzeugt, wenn Sie eine PDF-Datei wählen, enthält ebenfalls die Textmarken und Strukturtags. Werden mehrere Seiten auf einem Blatt zusammengefasst, verweist jede Textmarke und jeder Link auf das Blatt, das ihre Seite enthält. Die Seiten auf einem Blatt werden in der Reihenfolge gelesen, in der sie darauf liegen: von links nach rechts, dann von oben nach unten.

## Als Webseite speichern

Klicken Sie auf **Datei > Exportieren** und wählen Sie eine von zwei Arten von Webseiten. Eine **Webseite** ist eine Datei, die Sie in jedem Webbrowser öffnen können.

- **Als Webseite speichern** erstellt eine einzige .html-Datei mit Ihren Wörtern, ihrem Erscheinungsbild und Ihren Bildern. Sie können diese eine Datei allein versenden. Erhalten bleiben Schriftart, Schriftgrad, Farbe, Hervorhebung, fett, kursiv, unterstrichen, Ausrichtung und der Abstand um Absätze.
- **Als gefilterte Webseite speichern** erstellt eine kleine .html-Datei, die nur die Struktur enthält: Überschriften, fett und kursiv, Listen, Tabellen, Links und Bilder. Sie enthält keine Schriftarten, Schriftgrade oder Farben, sodass Sie ihr später eigene Gestaltung geben können.

Beide erhalten Überschriften, Aufzählungs- und nummerierte Listen, Tabellen mit verbundenen Zellen, Links und Bilder. Ausgeblendeter Text, Wörter, die eine nachverfolgte Änderung gelöscht hat, Kommentare, Kopf- und Fußzeilen sowie Notizen werden weggelassen. Ein Link, der ein Programm starten oder ein Skript ausführen würde, wird als reiner Text gespeichert.

## Als E-Book speichern

Ein **E-Book** ist ein Buch, das Sie auf einem E-Reader, Telefon oder Tablet lesen. Die Datei ist ein **EPUB**. Der Text passt sich jedem Bildschirm an, sodass der Leser die Schrift größer oder kleiner stellen kann.

1. Klicken Sie auf **Datei > Exportieren > Als E-Book (EPUB) speichern**.
2. Prüfen Sie **Titel** und **Autor**. Sie stammen zunächst aus den Eigenschaften Ihres Dokuments (**Datei > Info**). Sie können sie nur für dieses Buch ändern.
3. Aktivieren Sie **Erstes Bild als Titelbild verwenden**, wenn das erste Bild Ihres Dokuments das Titelbild sein soll. Ohne diese Option hat das Buch kein Titelbild.
4. Klicken Sie auf **OK** und wählen Sie dann den Speicherort der Datei.

So entsteht das Buch:

- Jede **Überschrift 1** beginnt ein neues Kapitel. Der Text vor der ersten Überschrift 1 bildet ein eigenes Kapitel.
- Das **Inhaltsverzeichnis** zeigt jede Überschrift 1, 2 und 3, und der Leser kann zu jeder springen. Kleinere Überschriften sind im Buch enthalten, aber nicht im Inhaltsverzeichnis.
- Bilder und Tabellen bleiben dort, wo Sie sie platziert haben. Der Alternativtext eines Bildes wird mit übernommen, damit ein Screenreader beschreiben kann, was es zeigt.
- Fußnoten und Endnoten stehen am Ende des Kapitels, in dem sie sich befinden. Jede Notiz enthält einen Link zurück zur Textstelle, und jede Nummer im Text ist ein Link zu ihrer Notiz.

Ein Buch ist begrenzt, damit eine beschädigte Datei kein riesiges Buch erzeugen kann: bis zu 500 Kapitel, 4.000 Zeilen im Inhaltsverzeichnis, 1.000 Bilder (keines größer als 32 MB) und 20.000 Notizen. Was darüber hinausgeht, bleibt im letzten Kapitel oder wird zu reinem Text, und Ihre Wörter gehen nie verloren: Ein Bild über dem Grenzwert wird durch seinen Alternativtext ersetzt, und eine Notiz über dem Grenzwert erhält kein Erscheinungsbild. Ein Bild, das ein Lesegerät nicht anzeigen kann (eine Metadatei, eine Bitmap oder ein TIFF), wird in ein PNG umgewandelt. Wenn Sie dasselbe gespeicherte Dokument erneut exportieren, entsteht dasselbe Buch, sodass ein Leser seine Position behält.

## Ein PDF mit Kennwort schützen

Klicken Sie im Feld **PDF exportieren** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer das PDF öffnet, muss es dann eingeben. Das Kennwort schützt die Datei tatsächlich: Die Seiten werden mit starker Verschlüsselung (AES-256) unlesbar gemacht, und ohne Kennwort kann sie niemand lesen.

Sie können außerdem **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Reader beachten sie, aber wer sie umgehen will, wird dadurch nicht gehindert. Verlassen Sie sich deshalb nicht darauf, um eine Datei privat zu halten.

Ein vergessenes Kennwort lässt sich nicht wiederherstellen. Bewahren Sie es daher sicher auf. Das Kennwort gilt nur für dieses eine PDF und wird nirgends gespeichert. Solange ein Kennwort vergeben ist, ist **PDF/A** ausgeschaltet, weil eine PDF/A-Datei kein Kennwort haben kann.

## Ein PDF/A erstellen

Aktivieren Sie im Feld **PDF exportieren** die Option **PDF/A zur langfristigen Aufbewahrung**. Eine PDF/A-Datei (PDF/A-1b) enthält jede Schriftart, die sie verwendet, benennt ihre Farben und folgt einer Norm für die Aufbewahrung über viele Jahre. Sie kann für Screenreader gekennzeichnet werden, aber kein Kennwort haben.

## Eine E-Mail erstellen

Klicken Sie auf **Datei > Exportieren > E-Mail erstellen**, um eine Nachrichtendatei (.eml) zu erstellen. Es wird nichts gesendet. Sie wählen, was in die Nachricht kommt: das Dokument, das Dokument als PDF oder die angezeigten Seiten als Bilder in der Nachricht (die ersten 30 Seiten). Öffnen Sie die Datei in Ihrem E-Mail-Programm, tragen Sie die Empfänger ein und senden Sie sie. Bei einem PDF können Sie im Feld auf **Mit Kennwort verschlüsseln…** klicken. Teilen Sie das Kennwort dem Empfänger auf einem anderen Weg mit, nicht in der Nachricht selbst.

## Schriftarten in der Datei

Eine .docx-Datei kann die Schriftarten mitführen, in denen ihr Text gesetzt ist. So sieht das Dokument auch auf einem Computer, auf dem diese Schriftarten fehlen, so aus, wie sein Autor es gestaltet hat. JeffWrite liest sie beim Öffnen der Datei. Sie werden nur für dieses Dokument verwendet, solange es geöffnet ist. Sie werden nie auf Ihrem Computer installiert, kein anderes Dokument und kein anderes Programm kann sie sehen, und sie sind verschwunden, sobald Sie das Dokument schließen. Eine Schriftart, die die Datei mitbringt, gilt nicht als fehlende Schriftart.

JeffWrite verwendet keine Schriftart, deren Lizenz das Einbetten untersagt, keine beschädigte Schriftart und keine Schriftart über den ersten 64 in einer Datei. Tritt das ein, sagt die Meldung beim Öffnen der Datei es, und der Text wird stattdessen in einer ähnlichen Schriftart angezeigt. Die Datei behält beim Speichern ihre Schriftarten so, wie sie waren.

Um die Schriftarten, die Ihr Text verwendet, in den von Ihnen gespeicherten Dateien mitzuspeichern, klicken Sie auf **Datei > Optionen**, öffnen die Seite **Speichern** und aktivieren **Schriftarten in der Datei speichern**. Die Option ist zunächst ausgeschaltet. JeffWrite nimmt beim Speichern einer .docx-Datei dann die Schriftarten auf, die mit dem Programm mitgeliefert werden und die der Text verwendet, und zwar in jedem vom Dokument verwendeten Schnitt. Eine Schriftart, deren Lizenz das nicht erlaubt, nimmt es nie auf; wird eine ausgelassen, nennt die Meldung nach dem Speichern ihren Namen. Die Datei wird dadurch größer; in eine Datei passen höchstens 64 Schriftarten und 256 MB.

## Siehe auch

- [Willkommen bei JeffWrite](start)
- [Drucken](print)
- [JeffWrite mit der Tastatur bedienen](keyboard)
