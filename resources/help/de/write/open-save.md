# Öffnen, Speichern, Drucken und Exportieren
<!-- keywords: Schriftarten in der Datei, eingebettete Schriftarten, fehlende Schriftart, E-Book, Ebook, EPUB, E-Reader, Kapitel, Inhalt, Webseite, HTML, als Webseite speichern, gefilterte Webseite, E-Mail, eml, Textmarken, PDF-Lesezeichen, Umschalt+F5, letzte Bearbeitung, wo Sie aufgehört haben, integrierte Vorlagen, Katalog, Brief, Bericht, Kurzinformation, Manuskript, Newsletter, öffnen, speichern, speichern unter, docx, doc, dot, odt, OpenDocument, RTF, ältere Dateien, Vorlage, dotx, zuletzt verwendet, anheften, neu, schließen, drucken, PDF, exportieren, wiederherstellen, automatische Wiederherstellung, nicht gespeichert, Absturz, Dateiansicht, Dateiformate, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ott, reparieren, öffnen und reparieren, beschädigt, Kennwort, verschlüsseln, mit Kennwort schützen -->

Klicken Sie oben links auf **Datei**, um die Dateiansicht zu öffnen. Sie hat die Seiten Neu, Öffnen, Drucken und Exportieren sowie die Befehle Speichern und Speichern unter. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Ein neues Dokument beginnen

Drücken Sie **Strg+N**, oder klicken Sie auf **Datei > Neu** und dann auf **Leeres Dokument**. Ein neues Dokument öffnet sich in einem eigenen Fenster. Ist das aktuelle Fenster leer und wurde noch nie gespeichert, ersetzt das neue Dokument es.

## Ein Dokument öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen**. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. JeffWrite öffnet diese Dateiarten:

- **.docx** ist ein Dokument.
- **.dotx** ist eine Vorlage. Eine Vorlage öffnet sich als neues Dokument, das auf ihr beruht, sodass die Vorlage unverändert bleibt.
- **.docm** und **.dotm** sind ein Dokument und eine Vorlage, die **Makros** enthalten können, also kleine Programme, die zu einer Datei gehören. JeffWrite führt sie nie aus. Beim Speichern als .docm oder .dotm bleiben sie erhalten. Speichern Sie als .docx oder .odt, werden die Makros weggelassen, und JeffWrite weist Sie darauf hin.
- **.txt** ist reiner Text, nur die Wörter. Ist die Datei reines ASCII oder Unicode, liest JeffWrite sie sofort ein. Ist dies nicht der Fall, fragt JeffWrite nach der Codierung, in der sie gelesen werden soll, und zeigt die ersten Zeilen an, damit Sie prüfen können, ob die Buchstaben stimmen. Eine **Codierung** legt fest, wie eine Datei Buchstaben speichert, etwa UTF-8 oder Windows 1252. Beim Speichern als .txt wählen Sie die Codierung und die Zeilenenden. Die Datei enthält nur die Wörter, also keine Schriftarten, Bilder oder Tabellenlinien.
- **.html** und **.htm** sind Webseiten. JeffWrite liest die Wörter, Überschriften, fette, kursive und unterstrichene Schrift, Farben, Listen und Tabellen. Aufzählungszeichen und Nummern von Listen werden als eingegebene Zeichen übernommen. Bilder und Links werden weggelassen. JeffWrite lädt nie etwas aus dem Internet und ignoriert Skripte. Eine Webseite wird nie überschrieben: Speichern fragt nach einem neuen Namen und speichert eine .docx-Datei. Informationen zum Erstellen einer Webseite finden Sie unten unter **Als Webseite speichern**.
- **.doc** und **.dot** sind die älteren Dokument- und Vorlagenformate. JeffWrite liest sie, und Ihre Änderungen speichert es als neue .docx-Datei.
- **.odt**, **.ott** und **.fodt** sind OpenDocument-Dateien. JeffWrite liest alle drei. Es speichert .odt-Dateien, und Änderungen an einer .ott- oder .fodt-Datei speichert es als neue .odt-Datei. Siehe [Öffnen und Speichern von OpenDocument-Dateien](open-odt).
- **.rtf** ist Rich Text Format. JeffWrite liest und speichert es. Siehe [Öffnen von RTF-Dateien](open-rtf).

Die Seite **Öffnen** listet auch die Dokumente auf, die Sie zuletzt geöffnet oder gespeichert haben. Klicken Sie auf eines, um es zu öffnen. Mit der rechten Maustaste können Sie ein Dokument oben in der Liste anheften oder es aus der Liste entfernen.

Enthält eine Datei Teile, die JeffWrite noch nicht bearbeiten kann, etwa das Layout einer Tabelle oder eine Kopfzeile, bleiben sie erhalten. Siehe [Willkommen bei JeffWrite](start). Kann ein Teil einer Datei gar nicht gelesen werden, teilt JeffWrite mit, was es weggelassen hat. Dann wird die Datei nicht überschrieben: Speichern fragt nach einem neuen Namen, damit das Original behält, was die Kopie verloren hat.

## Ein Dokument speichern

Drücken Sie **Strg+S**, um zu speichern. Beim ersten Mal fragt JeffWrite nach einem Namen. **Speichern unter** (F12) speichert eine Kopie unter einem neuen Namen. Wählen Sie **Vorlage (*.dotx)**, um eine Vorlage zu speichern, **Dokument mit Makros (*.docm)**, um Makros zu behalten, **Nur-Text (*.txt)** für reine Wörter oder **Rich Text Format (*.rtf)** für eine Datei, die andere Programme lesen können. Eine Datei, die aus einem älteren Format stammt, wird immer unter einem neuen Namen gespeichert, sodass die alte Datei nie überschrieben wird.

## Eine Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**. Wählen Sie einen Namen. JeffWrite schreibt das Dokument in diese Datei, und Sie arbeiten im bisherigen Dokument weiter. Name und nicht gespeicherte Änderungen bleiben unverändert. Das ist praktisch, wenn Sie eine Fassung an jemanden senden und weiterschreiben möchten.

## Eine Sicherungskopie behalten

Klicken Sie auf **Datei > Optionen** und schalten Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen** ein. Bei jedem Speichern kopiert JeffWrite zuerst die alte Datei in eine Datei mit dem Namen „Sicherung von“ plus dem bisherigen Namen, im selben Ordner. Kann die Kopie nicht erstellt werden, fragt JeffWrite, ob ohne Sicherungskopie gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja sagen, und die Datei bleibt unverändert. So können Sie zur zuletzt gespeicherten Fassung zurückkehren.

## Als Vorlage speichern

Eine Vorlage ist ein Dokument, mit dem Sie beginnen. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage wird unter **Meine Vorlagen** abgelegt, das Dokument, das Sie gerade schreiben, bleibt unverändert. Im Dialog **Speichern unter** können Sie auch **Vorlage (*.dotx)** oder **OpenDocument-Textvorlage (*.ott)** wählen, um eine Vorlagendatei an einem Ort Ihrer Wahl zu speichern.

Um ein neues Dokument aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen stehen unter **Meine Vorlagen**. Klicken Sie auf eine davon. Ein neues Dokument öffnet sich mit dem Text und den Formatvorlagen der Vorlage. Um eine Vorlage aus der Liste zu entfernen, klicken Sie mit der rechten Maustaste darauf und wählen Sie **Aus Meine Vorlagen entfernen**.

## Andere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken. Jede Möglichkeit fragt nach einer Datei oder verwendet die, auf die Sie geklickt haben:

- **Schreibgeschützt öffnen** öffnet die Datei nur zum Lesen. Sie können sie ansehen, blättern, suchen und kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** wählen, fragt JeffWrite nach einem neuen Namen, sodass die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet ein neues Dokument mit dem Text der Datei. Es hat keine eigene Datei, daher fragt **Speichern**, wo es abgelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .docx- oder .doc-Datei, etwa eine, die beim Kopieren abgebrochen wurde. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .docx- oder .doc-Datei beschädigt. Ein Kopiervorgang wurde auf halbem Weg abgebrochen, oder eine Festplatte hat Fehler gemacht. Eine .docx-Datei ist ein Bündel kleinerer Teile, etwa des Textes, der Formatvorlagen und der Bilder. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten im Vorgang abbrach, wird bis zu der Stelle übernommen, an der er abbrach. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verbindungen zwischen ihnen, stellt JeffWrite sie neu her.

Eine alte .doc-Datei ist eine große Hülle mit einigen Datenströmen darin. Ein Strom enthält den Text, ein anderer die Tabelle, die angibt, wo jedes Textstück steht und wie es aussieht. Ist die Hülle abgeschnitten oder sind ihre Listen beschädigt, sucht **Öffnen und reparieren** diese Ströme anhand ihres Inhalts und holt so viel wie möglich heraus. Sind die Listen beschädigt, die festlegen, wie die Teile zusammengehören, nimmt das Programm jeden Teil als Fortsetzung des vorherigen und zeigt an, was es dort findet. Dieser Text könnte zu einer älteren Fassung des Dokuments oder zu einem anderen Teil der Datei gehören. Deshalb nennt die Meldung den Absatz, an dem er beginnt, und bittet Sie, ihn mit einer Sicherungskopie zu vergleichen, falls Sie eine haben. Liegt die Vermutung vor dem Text, meldet die Meldung, dass das gesamte Dokument neu zusammengesetzt wurde. Liegt sie nach dem Text, meldet sie, dass die Formatierung neu zusammengesetzt wurde. Konnten Bilder nicht vollständig wiederhergestellt werden, wird das ebenfalls gemeldet. Ist die Tabelle verloren, bleibt der Text als einfache Absätze ohne Formatvorlagen, Listen, Tabellen oder Bilder erhalten. Eine .doc-Datei mit Kennwort lässt sich nicht reparieren.

Das reparierte Dokument öffnet sich als Kopie mit einem Namen wie „Repariert Brief“. Eine Meldung sagt Ihnen, was bis zu einem Bruch erhalten blieb und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Kopie unter einem eigenen Namen und prüfen Sie sie, bevor Sie ihr vertrauen.

## Nicht gespeicherte Arbeit wiederherstellen

JeffWrite speichert alle paar Minuten eine Reservekopie nicht gespeicherter Arbeit als Datei in einem eigenen Ordner. Wird das Programm unerwartet beendet, bietet es diese Kopien beim nächsten Start an. Sie können sie später über **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen** ansehen. Sie wählen aus, welche Sie öffnen möchten, und können jede an einem Ort Ihrer Wahl speichern.

## Von einer Vorlage beginnen

**Datei > Neu** zeigt das **Leere Dokument** und eine Reihe mit JeffWrite gelieferter Vorlagen, nach Art gruppiert: einen Brief, einen Bericht, eine Kurzinformation, ein Manuskript und einen Newsletter. Jedes Bild zeigt die erste Seite des Dokuments, das Sie erhalten. Eine **Vorlage** ist ein Dokument, das bereits eingerichtet ist, mit Formatvorlagen, Seitenrändern und Beispieltext. Klicken Sie auf eine und ersetzen Sie den Beispieltext durch Ihren eigenen. Selbst gespeicherte Vorlagen stehen unter **Meine Vorlagen**.

## Dort weitermachen, wo Sie aufgehört haben

JeffWrite merkt sich, wo sich die Einfügemarke befand, als Sie eine Datei gespeichert oder geschlossen haben. Wenn Sie die Datei das nächste Mal öffnen, steht die Einfügemarke dort. Mit **Umschalt+F5** springen Sie zu den Stellen, an denen Sie zuletzt bearbeitet haben. Jeder Druck führt zur vorherigen Stelle: zuerst die letzten drei Stellen, dann zurück zur ersten.

## Als .odt speichern

Eine .odt-Datei kann nicht alles enthalten, was eine .docx-Datei enthalten kann. Beim Speichern als .odt listet JeffWrite zuerst auf, was verloren ginge, und lässt Sie stattdessen eine .docx-Datei speichern. Siehe [Info](info).

## Drucken

Drücken Sie **Strg+P**, oder klicken Sie auf **Datei > Drucken**. Wählen Sie einen Drucker, die Anzahl der Kopien und die Seiten, sehen Sie sich jede Seite in der Vorschau an und klicken Sie auf **Drucken**. Siehe [Drucken](print).

## Eine PDF-Datei erstellen

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Die PDF-Datei übernimmt Titel, Autor, Thema und Stichwörter aus **Datei > Info** (bei aktivierter Option **Dokumenteigenschaften einschließen**), das Erstellungsdatum und JeffWrite als das Programm, das sie erstellt hat. Der Autor und ein aus dem Dateinamen gebildeter Titel bleiben weg, wenn persönliche Informationen entfernt werden. Jeder dieser Werte wird bis zu 4.000 Zeichen vollständig übernommen. Aktivieren Sie **PDF nach dem Speichern öffnen**, damit die Datei beim Speichern im PDF-Programm Ihres Computers geöffnet wird. Die Option ist zunächst ausgeschaltet. Lässt sich die Datei auf Ihrem Computer nicht öffnen, zeigt die Statusleiste das an; die Datei ist trotzdem gespeichert. Wählen Sie, welche Seiten enthalten sein sollen: alle, die aktuelle Seite oder einen Bereich wie 1-3, 5 (auch p2s3 und s3 sind möglich). Wählen Sie dann, wie die Datei erstellt wird:

- **Optimieren für** bietet **Standard**, das Ihre Bilder unverändert lässt, und **Minimale Größe**, das jedes Bild auf höchstens 150 Pixel pro Zoll reduziert, damit die Datei kleiner wird. Verwenden Sie die minimale Größe für eine PDF-Datei, die Sie auf eine Webseite stellen oder per E-Mail verschicken möchten.
- **Dokumenteigenschaften einschließen** übernimmt Titel, Autor, Thema und Stichwörter des Dokuments in die PDF-Datei.
- **Textmarken aus Überschriften erstellen** macht aus jeder Überschrift eine **Textmarke**, also einen Eintrag in der Liste, die ein PDF-Reader neben den Seiten anzeigt. Klicken Sie auf eine Textmarke, um zu ihrer Seite zu springen.
- **Dokumentstruktur-Tags für Barrierefreiheit** kennzeichnet Überschriften, Absätze, Listen, Tabellen (mit ihren Kopfzeilen) und Bilder (mit ihrem Alternativtext) und legt die Sprache fest, damit ein Vorleseprogramm die PDF-Datei in der richtigen Reihenfolge liest.

Die PDF-Datei sieht auf jedem Computer gleich aus, weil die Schriftarten in ihr eingebettet sind. Die PDF-Datei, die Drucken erstellt, wenn Sie eine PDF-Datei wählen, enthält ebenfalls die Textmarken und Tags. Werden auf einem Blatt mehrere Seiten gedruckt, führt jede Textmarke und jeder Link zu dem Blatt, auf dem seine Seite liegt. Die Seiten auf einem Blatt werden in der Reihenfolge gelesen, in der sie liegen: von links nach rechts, dann von oben nach unten.

## Als Webseite speichern

Klicken Sie auf **Datei > Exportieren** und wählen Sie eine von zwei Arten von Webseiten. Eine **Webseite** ist eine Datei, die Sie in jedem Webbrowser öffnen können.

- **Als Webseite speichern** erstellt eine einzige .html-Datei mit Ihrem Text, seinem Erscheinungsbild und Ihren Bildern. Sie können diese eine Datei allein versenden. Erhalten bleiben Schriftart, Schriftgrad, Farbe, Hervorhebung, Fett, Kursiv, Unterstreichung, Ausrichtung und der Abstand zwischen Absätzen.
- **Als gefilterte Webseite speichern** erstellt eine kleine .html-Datei, die nur die Struktur enthält: Überschriften, fette und kursive Schrift, Listen, Tabellen, Links und Bilder. Sie enthält keine Schriftarten, Schriftgrade oder Farben, sodass Sie ihr später ein eigenes Erscheinungsbild geben können.

Beide Varianten erhalten Überschriften, Aufzählungen und nummerierte Listen, Tabellen mit verbundenen Zellen, Links und Bilder. Ausgeblendeter Text, Wörter, die eine nachverfolgte Änderung gelöscht hat, Kommentare, Kopfzeilen, Fußzeilen und Notizen werden weggelassen. Ein Link, der ein Programm starten oder ein Skript ausführen würde, wird als reiner Text gespeichert.

## Als E-Book speichern

Ein **E-Book** ist ein Buch, das Sie auf einem E-Reader, einem Smartphone oder einem Tablet lesen. Die Datei ist ein **EPUB**. Der Text passt sich jedem Bildschirm an, sodass die Leserin oder der Leser die Schrift größer oder kleiner stellen kann.

1. Klicken Sie auf **Datei > Exportieren > Als E-Book (EPUB) speichern**.
2. Prüfen Sie **Titel** und **Autor**. Sie stammen zunächst aus den Eigenschaften Ihres Dokuments (**Datei > Info**). Sie können sie nur für dieses Buch ändern.
3. Aktivieren Sie **Erstes Bild als Titelbild verwenden**, wenn das erste Bild Ihres Dokuments das Titelbild sein soll. Ohne diese Option hat das Buch kein Titelbild.
4. Klicken Sie auf **OK** und wählen Sie dann den Speicherort der Datei.

So wird das Buch erstellt:

- Jede **Überschrift 1** beginnt ein neues Kapitel. Die Wörter vor der ersten Überschrift 1 bilden ein eigenes Kapitel.
- Das **Inhaltsverzeichnis** zeigt jede Überschrift 1, 2 und 3, und die Leserin oder der Leser kann zu jeder springen. Kleinere Überschriften sind im Buch enthalten, aber nicht im Inhaltsverzeichnis.
- Bilder und Tabellen bleiben dort, wo Sie sie platziert haben. Der Alternativtext eines Bildes wird mit ihm übernommen, damit ein Vorleseprogramm beschreiben kann, was es zeigt.
- Fuß- und Endnoten werden am Ende des Kapitels gesetzt, in dem sie stehen. Jede Notiz hat einen Link zurück zur Stelle im Text, und jede Nummer im Text ist ein Link zu ihrer Notiz.

Ein Buch unterliegt Grenzen, damit eine beschädigte Datei kein riesiges Buch erzeugen kann: bis zu 500 Kapitel, 4.000 Zeilen im Inhaltsverzeichnis, 1.000 Bilder (keines größer als 32 MB) und 20.000 Notizen. Was darüber hinausgeht, bleibt im letzten Kapitel oder wird zu reinem Text, und Ihr Text geht nie verloren: Ein Bild über der Grenze wird durch seinen Alternativtext ersetzt, und eine Notiz über der Grenze erhält kein Erscheinungsbild. Ein Bild, das ein Lesegerät nicht anzeigen kann (eine Metadatei, eine Bitmap oder ein TIFF), wird in ein PNG umgewandelt. Exportieren Sie dasselbe gespeicherte Dokument erneut, entsteht dasselbe Buch, sodass Leserinnen und Leser ihre Stelle behalten.

## Eine PDF-Datei mit Kennwort schützen

Klicken Sie im Feld **Als PDF exportieren** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer die PDF-Datei öffnet, muss dann das Kennwort eingeben. Das Kennwort schützt die Datei tatsächlich: Die Seiten werden mit starker Verschlüsselung (AES-256) unlesbar gemacht, und niemand kann sie ohne Kennwort lesen.

Sie können auch **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Reader beachten sie, doch sie hindern niemanden daran, sie zu umgehen. Verlassen Sie sich daher nicht darauf, eine Datei damit vertraulich zu halten.

Ein vergessenes Kennwort lässt sich nicht wiederherstellen, bewahren Sie es also an einem sicheren Ort auf. Das Kennwort gilt nur für diese eine PDF-Datei und wird nie gespeichert. Solange ein Kennwort festgelegt ist, ist **PDF/A** im Feld deaktiviert, da eine PDF/A-Datei kein Kennwort haben kann.

## Eine PDF/A-Datei erstellen

Aktivieren Sie im Feld **Als PDF exportieren** die Option **PDF/A zur langfristigen Aufbewahrung**. Eine PDF/A-Datei (PDF/A-1b) enthält alle verwendeten Schriftarten, benennt ihre Farben und folgt einer Norm für die Aufbewahrung von Dateien über viele Jahre. Sie kann für Vorleseprogramme mit Tags versehen werden, kann aber kein Kennwort haben.

## Eine E-Mail erstellen

Klicken Sie auf **Datei > Exportieren > E-Mail erstellen**, um eine Nachrichtendatei (.eml) zu erstellen. Es wird nichts gesendet. Sie wählen, was in die Nachricht kommt: das Dokument, das Dokument als PDF oder die Seiten als Bilder in der Nachricht (die ersten 30 Seiten). Öffnen Sie die Datei in Ihrem E-Mail-Programm, tragen Sie die Empfängerin oder den Empfänger ein und senden Sie sie. Für eine PDF-Datei können Sie im Feld auf **Mit Kennwort verschlüsseln…** klicken; teilen Sie das Kennwort auf anderem Weg mit und nicht in der Nachricht.

## Schriftarten in der Datei

Eine .docx-Datei kann die Schriftarten enthalten, in denen ihr Text gesetzt ist. So sieht das Dokument aus, wie sein Verfasser es gestaltet hat, auch auf einem Computer, auf dem diese Schriftarten fehlen. JeffWrite liest sie beim Öffnen der Datei ein. Sie werden nur für dieses Dokument und nur solange es geöffnet ist verwendet. Sie werden nie auf Ihrem Computer installiert, kein anderes Dokument und kein anderes Programm sieht sie, und sie sind nach dem Schließen des Dokuments verschwunden. Eine Schriftart, die die Datei mitbringt, gilt nicht als fehlende Schriftart.

JeffWrite verwendet keine Schriftart, deren Lizenz die Einbettung untersagt, keine beschädigte Schriftart und keine Schriftart über den ersten 64 in einer Datei. Tritt das ein, sagt die Meldung beim Öffnen der Datei dies, und der Text wird stattdessen in einer ähnlichen Schriftart angezeigt. Die Datei behält ihre Schriftarten so, wie sie beim Speichern waren.

Um die Schriftarten, die Ihr Text verwendet, in den gespeicherten .docx-Dateien mitzuspeichern, klicken Sie auf **Datei > Optionen**, öffnen Sie die Seite **Speichern** und aktivieren Sie **Schriftarten in der Datei speichern**. Die Option ist zunächst ausgeschaltet. JeffWrite nimmt beim Speichern einer .docx-Datei dann die Schriftarten mit auf, die es mitbringt und die der Text verwendet, in jedem Schnitt, den das Dokument benutzt. Eine Schriftart, deren Lizenz das nicht erlaubt, wird nie aufgenommen; wird eine ausgelassen, nennt die Meldung nach dem Speichern ihren Namen. Die Datei wird dadurch größer, und in einer Datei werden höchstens 64 Schriftarten und 256 MB aufgenommen.

## Siehe auch

- [Willkommen bei JeffWrite](start)
- [Drucken](print)
- [JeffWrite mit der Tastatur bedienen](keyboard)
