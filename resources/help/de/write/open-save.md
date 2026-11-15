# Öffnen, Speichern, Drucken und Exportieren
<!-- keywords: Webseite, html, als Webseite speichern, gefilterte Webseite, E-Mail, eml, Textmarken, PDF-Lesezeichen, umschalt+f5, letzte Bearbeitung, wo Sie aufgehört haben, integrierte Vorlagen, Katalog, Brief, Bericht, Kurzinformation, Manuskript, Newsletter, öffnen, speichern, speichern unter, docx, doc, dot, odt, OpenDocument, rtf, ältere Dateien, Vorlage, dotx, zuletzt verwendet, anheften, neu, schließen, drucken, PDF, exportieren, wiederherstellen, automatische Wiederherstellung, nicht gespeichert, Absturz, Dateiansicht, Dateiformate, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, meine Vorlagen, als Vorlage speichern, ott, reparieren, öffnen und reparieren, beschädigt, Kennwort, verschlüsseln, mit Kennwort schützen -->

Klicken Sie oben links auf **Datei**, um die Dateiansicht zu öffnen. Sie enthält die Seiten **Neu**, **Öffnen**, **Drucken** und **Exportieren** sowie die Befehle **Speichern** und **Speichern unter**. Mit **Esc** kehren Sie zu Ihrem Dokument zurück.

## Ein neues Dokument beginnen

Drücken Sie **Strg+N**, oder klicken Sie auf **Datei > Neu** und dann auf **Leeres Dokument**. Ein neues Dokument öffnet sich in einem eigenen Fenster. Ist das aktuelle Fenster leer und wurde noch nie gespeichert, ersetzt das neue Dokument es.

## Ein Dokument öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen**. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. JeffWrite öffnet diese Dateiarten:

- **.docx** ist ein Dokument.
- **.dotx** ist eine Vorlage. Eine Vorlage öffnet sich als neues Dokument, das auf ihr basiert, sodass die Vorlage unverändert bleibt.
- **.docm** und **.dotm** sind ein Dokument und eine Vorlage, die **Makros** enthalten können, kleine Programme, die zu einer Datei gehören. JeffWrite führt sie nie aus. Beim Speichern als .docm oder .dotm bleiben sie erhalten. Beim Speichern als .docx oder .odt werden die Makros weggelassen, und JeffWrite weist Sie darauf hin.
- **.txt** ist reiner Text, nur die Wörter. Ist die Datei reines ASCII oder Unicode, liest JeffWrite sie sofort ein. Ist dies nicht der Fall, fragt JeffWrite nach der **Codierung**, mit der gelesen werden soll, und zeigt die ersten Zeilen an, damit Sie sehen, ob die Buchstaben stimmen. Eine Codierung legt fest, wie eine Datei Buchstaben speichert, etwa UTF-8 oder Windows-1252. Beim Speichern als .txt wählen Sie die Codierung und die Zeilenenden. Die Datei enthält nur die Wörter: keine Schriftarten, Bilder oder Tabellenlinien.
- **.html** und **.htm** sind Webseiten. JeffWrite liest die Wörter, Überschriften, fett, kursiv, unterstrichen, Farben, Listen und Tabellen. Aufzählungszeichen und Nummern von Listen werden als eingegebene Zeichen übernommen; Bilder und Links werden weggelassen. JeffWrite lädt nichts aus dem Internet und ignoriert Skripte. Eine Webseite wird nie überschrieben: **Speichern** fragt nach einem neuen Namen und speichert eine .docx-Datei. Informationen zum Erstellen einer Webseite finden Sie unten unter **Als Webseite speichern**.
- **.doc** und **.dot** sind die älteren Dokument- und Vorlagenformate. JeffWrite liest sie, und Ihre Änderungen speichert es als neue .docx-Datei.
- **.odt**, **.ott** und **.fodt** sind OpenDocument-Dateien. JeffWrite liest alle drei Formate. Es speichert .odt-Dateien, und Änderungen an einer .ott- oder .fodt-Datei speichert es als neue .odt-Datei. Siehe [Open and save OpenDocument files](open-odt).
- **.rtf** ist Rich Text Format. JeffWrite liest und speichert es. Siehe [Open RTF files](open-rtf).

Auf der Seite **Öffnen** werden auch die zuletzt geöffneten oder gespeicherten Dokumente aufgelistet. Klicken Sie auf einen Eintrag, um ihn zu öffnen. Mit einem Rechtsklick auf einen Eintrag heften Sie ihn oben in der Liste an oder entfernen ihn aus der Liste.

Enthält eine Datei Teile, die JeffWrite noch nicht bearbeiten kann, etwa das Layout einer Tabelle oder eine Kopfzeile, bleiben sie erhalten. Siehe [Welcome to JeffWrite](start). Lässt sich ein Teil einer Datei gar nicht lesen, teilt JeffWrite mit, was es weggelassen hat. Danach wird die Datei nicht überschrieben: **Speichern** fragt nach einem neuen Namen, damit das Original behält, was die Kopie verloren hat.

## Ein Dokument speichern

Drücken Sie **Strg+S**, um zu speichern. Beim ersten Mal fragt JeffWrite nach einem Namen. **Speichern unter** (**F12**) speichert eine Kopie unter einem neuen Namen. Wählen Sie **Vorlage (*.dotx)**, um eine Vorlage zu speichern, **Dokument mit Makros (*.docm)**, um Makros zu behalten, **Nur-Text (*.txt)** für reinen Text oder **Rich Text Format (*.rtf)** für eine Datei, die andere Programme lesen können. Eine Datei, die aus einem älteren Dateiformat stammt, wird immer unter einem neuen Namen gespeichert, damit die alte Datei nie überschrieben wird.

## Eine Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**. Wählen Sie einen Namen. JeffWrite schreibt das Dokument in diese Datei, und Sie arbeiten im bisherigen Dokument weiter. Name und nicht gespeicherte Änderungen bleiben unverändert. Das ist praktisch, wenn Sie eine Fassung an jemanden senden und weiterschreiben möchten.

## Eine Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen** und aktivieren Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffWrite zuerst die alte Datei in eine Datei mit dem Namen „Sicherung von“ plus ursprünglicher Name im selben Ordner. Lässt sich die Kopie nicht erstellen, fragt JeffWrite, ob ohne Sicherung gespeichert werden soll. Die Antwort lautet Nein, sofern Sie nicht Ja sagen; dann bleibt die Datei unverändert. So können Sie zur zuletzt gespeicherten Fassung zurückkehren.

## Als Vorlage speichern

Eine Vorlage ist ein Dokument, das Sie als Ausgangspunkt verwenden. Klicken Sie auf **Datei > Als Vorlage speichern**, und geben Sie einen Namen ein. Die Vorlage landet unter **Meine Vorlagen**, und das Dokument, an dem Sie gerade arbeiten, bleibt unverändert. Im Dialog **Speichern unter** können Sie auch **Vorlage (*.dotx)** oder **OpenDocument-Textvorlage (*.ott)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um ein neues Dokument aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen sind unter **Meine Vorlagen** aufgelistet. Klicken Sie eine an. Ein neues Dokument mit Text und Formatvorlagen der Vorlage öffnet sich. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Andere Möglichkeiten zum Öffnen einer Datei

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste einen Namen in der Liste der zuletzt verwendeten Dateien anklicken. Jede dieser Möglichkeiten fragt nach einer Datei oder verwendet die Datei, auf die Sie geklickt haben:

- **Schreibgeschützt öffnen** öffnet die Datei nur zum Lesen. Sie können ansehen, blättern, suchen und kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Drücken Sie **Speichern**, fragt JeffWrite nach einem neuen Namen, sodass die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet ein neues Dokument mit dem Text der Datei. Es hat keine eigene Datei, daher fragt **Speichern**, wohin es soll. Die geöffnete Datei wird nie geändert.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .docx-Datei, etwa eine, die beim Kopieren abgeschnitten wurde. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .docx-Datei beschädigt, etwa weil ein Kopiervorgang auf halbem Weg abbrach oder ein Datenträger Fehler gemacht hat. Eine .docx-Datei ist ein Bündel kleinerer Teile, etwa des Textes, der Formatvorlagen und der Bilder. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten im Vorgang abbrach, wird bis zu der Stelle übernommen, an der er abbrach. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verbindungen zwischen ihnen, stellt JeffWrite sie neu her.

Das reparierte Dokument öffnet sich als Kopie mit einem Namen wie „Repariert Brief“. Eine Meldung informiert Sie, was bis zu einem Abbruch übernommen und was weggelassen wurde. Die beschädigte Datei bleibt unverändert. Speichern Sie die reparierte Kopie unter einem eigenen Namen, und prüfen Sie sie, bevor Sie ihr vertrauen.

## Nicht gespeicherte Arbeit wiederherstellen

JeffWrite speichert alle paar Minuten eine Sicherungskopie nicht gespeicherter Arbeit als Datei in einem eigenen Ordner. Wird das Programm unerwartet geschlossen, bietet es diese Kopien beim nächsten Start an. Über **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen** sehen Sie sie auch später. Sie wählen aus, welche Sie öffnen möchten, und können jede an einem beliebigen Ort speichern.

## Mit einer Vorlage beginnen

**Datei > Neu** zeigt das **Leere Dokument** und eine Auswahl an Vorlagen, die JeffWrite mitbringt, nach Art sortiert: Brief, Bericht, Kurzinformation, Manuskript und Newsletter. Jedes Bild zeigt die erste Seite des Dokuments, das Sie erhalten. Eine **Vorlage** ist ein fertig eingerichtetes Dokument mit Formatvorlagen, Seitenrändern und Beispieltexten. Klicken Sie eine an und ersetzen Sie die Beispieltexte durch Ihre eigenen. Selbst gespeicherte Vorlagen finden Sie unter **Meine Vorlagen**.

## Dort weitermachen, wo Sie aufgehört haben

JeffWrite merkt sich, wo der Cursor beim Speichern oder Schließen einer Datei stand. Beim nächsten Öffnen der Datei steht der Cursor an derselben Stelle. Mit **Umschalt+F5** springen Sie zu den Stellen, die Sie zuletzt bearbeitet haben. Jeder Druck führt zur vorherigen Stelle: zuerst zu den letzten drei Stellen, dann wieder zur ersten.

## Als .odt speichern

Eine .odt-Datei kann nicht alles enthalten, was eine .docx-Datei kann. Beim Speichern als .odt listet JeffWrite zuerst auf, was verloren ginge, und bietet an, stattdessen eine .docx-Datei zu speichern. Siehe [Info](info).

## Drucken

Drücken Sie **Strg+P**, oder klicken Sie auf **Datei > Drucken**. Wählen Sie einen Drucker, die Anzahl der Kopien und die Seiten, sehen Sie sich jede Seite in der Vorschau an und klicken Sie auf **Drucken**. Siehe [Print](print).

## Eine PDF-Datei erstellen

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Wählen Sie, welche Seiten aufgenommen werden: alle, die aktuelle Seite oder ein Bereich wie 1-3, 5 (p2s3 und s3 funktionieren hier ebenfalls). Legen Sie dann fest, wie die Datei erstellt wird:

- **Optimieren für** bietet **Standard**, das Ihre Bilder unverändert lässt, und **Minimale Größe**, das jedes Bild auf höchstens 150 Pixel pro Zoll reduziert, damit die Datei kleiner wird. Verwenden Sie die minimale Größe für ein PDF, das Sie auf eine Webseite stellen oder per E-Mail verschicken.
- **Dokumenteigenschaften einschließen** übernimmt Titel, Autor, Betreff und Stichwörter des Dokuments in das PDF.
- **Textmarken aus Überschriften erstellen** macht aus jeder Überschrift eine Textmarke, also einen Eintrag in der Liste, die ein PDF-Reader neben den Seiten anzeigt. Klicken Sie auf eine Textmarke, um zu ihrer Seite zu springen.
- **Dokumentstruktur-Tags für Barrierefreiheit** kennzeichnet Überschriften, Absätze, Listen, Tabellen (mit ihren Kopfzeilen) und Bilder (mit ihrem Alternativtext) und legt die Sprache fest, damit ein Vorlesegerät das PDF in der richtigen Reihenfolge liest.

Wählen Sie danach einen Namen. Das PDF sieht auf jedem Computer gleich aus, weil die Schriftarten in der Datei enthalten sind. Auch das PDF, das **Drucken** erstellt, wenn Sie eine PDF-Datei wählen, enthält Textmarken und Strukturtags.

## Als Webseite speichern

Klicken Sie auf **Datei > Exportieren** und wählen Sie eine von zwei Arten von Webseiten. Eine Webseite ist eine Datei, die Sie in jedem Webbrowser öffnen können.

- **Als Webseite speichern** erstellt eine einzelne .html-Datei, die Ihre Wörter, deren Aussehen und Ihre Bilder enthält. Diese eine Datei können Sie allein versenden. Das behaltene Aussehen umfasst Schriftart, Schriftgrad, Farbe, Hervorhebung, fett, kursiv, unterstrichen, Ausrichtung und den Abstand zwischen Absätzen.
- **Als gefilterte Webseite speichern** erstellt eine kleine .html-Datei, die nur die Struktur enthält. Sie enthält keine Schriftarten, Schriftgrade oder Farben, sodass Sie später ein eigenes Aussehen festlegen können.

Beide behalten Überschriften, Aufzählungs- und nummerierte Listen, Tabellen mit verbundenen Zellen, Links und Bilder bei. Ausgeblendeter Text, Wörter, die eine nachverfolgte Änderung gelöscht hat, Kommentare, Kopf- und Fußzeilen sowie Notizen werden weggelassen. Ein Link, der ein Programm starten oder ein Skript ausführen würde, wird als reiner Text gespeichert.

## Eine PDF-Datei mit Kennwort schützen

Klicken Sie im Feld **Als PDF exportieren** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer das PDF öffnet, muss es dann eingeben. Das Kennwort schützt die Datei tatsächlich: Die Seiten werden mit starker Verschlüsselung (AES-256) unlesbar gemacht, und ohne Kennwort kann sie niemand lesen.

Sie können auch **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Reader beachten sie, aber sie halten niemanden davon ab, sie zu umgehen. Verlassen Sie sich deshalb nicht darauf, eine Datei damit vertraulich zu halten.

Ein vergessenes Kennwort lässt sich nicht wiederherstellen. Bewahren Sie es daher an einem sicheren Ort auf. Das Kennwort gilt nur für dieses eine PDF und wird nirgends gespeichert. Solange ein Kennwort gesetzt ist, ist **PDF/A** im Feld deaktiviert, weil eine PDF/A-Datei kein Kennwort haben kann.

## Eine PDF/A-Datei erstellen

Aktivieren Sie im Feld **Als PDF exportieren** die Option **PDF/A zur langfristigen Aufbewahrung**. Eine PDF/A-Datei (PDF/A-2b) enthält alle verwendeten Schriftarten, benennt ihre Farben und folgt einem Standard für die Aufbewahrung über viele Jahre. Sie kann für Vorlesegeräte gekennzeichnet werden, darf jedoch kein Kennwort haben.

## E-Mail erstellen

Klicken Sie auf **Datei > Exportieren > E-Mail erstellen**, um eine Nachrichtendatei (.eml) zu erstellen. Es wird nichts versendet. Sie legen fest, was in die Nachricht kommt: das Dokument, das Dokument als PDF oder die angezeigten Seiten als Bilder in der Nachricht (die ersten 30 Seiten). Öffnen Sie die Datei in Ihrem E-Mail-Programm, tragen Sie den Empfänger ein und senden Sie sie. Für ein PDF können Sie im Feld auf **Mit Kennwort verschlüsseln…** klicken; teilen Sie das Kennwort dem Empfänger auf anderem Weg mit, nicht in der Nachricht.

## Siehe auch

- [Welcome to JeffWrite](start)
- [Print](print)
- [Use JeffWrite with the keyboard](keyboard)
