# Öffnen, Speichern, Drucken und Exportieren
<!-- keywords: E-Mail, eml, Textmarken, PDF-Lesezeichen, Umschalt+F5, letzte Bearbeitung, wo Sie aufgehört haben, integrierte Vorlagen, Katalog, Brief, Bericht, Kurzinformation, Manuskript, Newsletter, öffnen, speichern, speichern unter, docx, doc, dot, odt, OpenDocument, rtf, ältere Dateien, Vorlage, dotx, zuletzt verwendet, anheften, neu, schließen, drucken, PDF, exportieren, wiederherstellen, automatische Wiederherstellung, nicht gespeichert, Absturz, Dateiansicht, Dateiformate, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ott, reparieren, öffnen und reparieren, beschädigt -->

Klicken Sie oben links auf **Datei**, um die Dateiansicht zu öffnen. Sie hat die Seiten Neu, Öffnen, Drucken und Exportieren sowie die Befehle Speichern und Speichern unter. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Ein neues Dokument beginnen

Drücken Sie **Strg+N**, oder klicken Sie auf **Datei > Neu** und dann auf **Leeres Dokument**. Ein neues Dokument öffnet sich in einem eigenen Fenster. Wenn das aktuelle Fenster leer ist und noch nie gespeichert wurde, ersetzt das neue Dokument es.

## Ein Dokument öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen**. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. JeffWrite öffnet diese Dateiarten:

- **.docx** ist ein Dokument.
- **.dotx** ist eine Vorlage. Eine Vorlage öffnet sich als neues Dokument, das auf ihr basiert, sodass die Vorlage unverändert bleibt.
- **.docm** und **.dotm** sind ein Dokument und eine Vorlage, die **Makros** enthalten können, also kleine Programme, die zu einer Datei gehören. JeffWrite führt sie nie aus. Beim Speichern als .docm oder .dotm bleiben sie erhalten. Wenn Sie als .docx oder .odt speichern, werden die Makros weggelassen, und JeffWrite weist Sie darauf hin.
- **.txt** ist Nur-Text, also nur die Wörter. Beim Öffnen liest JeffWrite die Datei sofort, wenn sie reines ASCII oder Unicode ist. Ist das nicht der Fall, fragt JeffWrite nach der **Codierung**, mit der die Datei gelesen werden soll, und zeigt die ersten Zeilen an, damit Sie prüfen können, ob die Buchstaben korrekt aussehen. Eine Codierung legt fest, wie eine Datei Buchstaben speichert, zum Beispiel UTF-8 oder Windows 1252. Beim Speichern als .txt wählen Sie die Codierung und die Zeilenenden. Die Datei enthält dann nur die Wörter: keine Schriftarten, Bilder oder Tabellenlinien.
- **.doc** und **.dot** sind die älteren Dokument- und Vorlagenformate. JeffWrite liest sie, und Ihre Änderungen speichert es als neue .docx-Datei.
- **.odt**, **.ott** und **.fodt** sind OpenDocument-Dateien. JeffWrite liest alle drei. Es speichert .odt-Dateien, und Änderungen an einer .ott- oder .fodt-Datei speichert es als neue .odt-Datei. Siehe [Open and save OpenDocument files](open-odt).
- **.rtf** ist Rich Text Format. JeffWrite liest und speichert es. Siehe [Open RTF files](open-rtf).

Auf der Seite **Öffnen** sind auch die Dokumente aufgeführt, die Sie zuletzt geöffnet oder gespeichert haben. Klicken Sie auf eines, um es zu öffnen. Mit einem Rechtsklick heften Sie es oben in der Liste an oder entfernen es aus der Liste.

Wenn eine Datei Teile enthält, die JeffWrite noch nicht bearbeiten kann, etwa das Layout einer Tabelle oder eine Kopfzeile, bleiben diese Teile erhalten. Siehe [Welcome to JeffWrite](start). Wenn ein Teil einer Datei gar nicht gelesen werden kann, teilt JeffWrite mit, was es weggelassen hat. Anschließend überschreibt es die Datei nicht: Beim Speichern fragt JeffWrite nach einem neuen Namen, damit die Originaldatei erhalten bleibt, die der Kopie fehlende Inhalte hat.

## Ein Dokument speichern

Drücken Sie **Strg+S**, um zu speichern. Beim ersten Mal fragt JeffWrite nach einem Namen. **Speichern unter** (F12) speichert eine Kopie unter einem neuen Namen. Wählen Sie **Vorlage (*.dotx)**, um eine Vorlage zu speichern, **Dokument mit Makros (*.docm)**, um Makros zu behalten, **Nur-Text (*.txt)** für reinen Text oder **Rich Text Format (*.rtf)** für eine Datei, die andere Programme lesen können. Eine Datei, die aus einem älteren Dateiformat stammt, wird immer unter einem neuen Namen gespeichert, damit die alte Datei nie überschrieben wird.

## Eine Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**. Wählen Sie einen Namen. JeffWrite schreibt das Dokument in diese Datei, und Sie arbeiten im bisherigen Dokument weiter. Dessen Name und nicht gespeicherte Änderungen bleiben unverändert. Das ist praktisch, wenn Sie eine Fassung verschicken und weiterschreiben möchten.

## Eine Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen**, und schalten Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen** ein. Bei jedem Speichern kopiert JeffWrite zuerst die alte Datei in eine Datei mit dem Namen „Sicherung von“ plus dem Dateinamen, im selben Ordner. Kann die Kopie nicht erstellt werden, fragt JeffWrite, ob ohne sie gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja sagen, und die Datei bleibt unverändert. So können Sie zur letzten gespeicherten Fassung zurückkehren.

## Als Vorlage speichern

Eine Vorlage ist ein Dokument, das Sie als Ausgangspunkt verwenden. Klicken Sie auf **Datei > Als Vorlage speichern**, und geben Sie einen Namen ein. Die Vorlage landet unter **Meine Vorlagen**, und das Dokument, an dem Sie schreiben, wird nicht verändert. Unter **Speichern unter** können Sie auch **Vorlage (*.dotx)** oder **OpenDocument-Textvorlage (*.ott)** wählen, um eine Vorlagendatei an einem Ort Ihrer Wahl zu speichern.

Um ein neues Dokument aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen sind unter **Meine Vorlagen** aufgeführt. Klicken Sie auf eine davon. Ein neues Dokument öffnet sich mit dem Text und den Formatvorlagen der Vorlage. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Andere Möglichkeiten, eine Datei zu öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Außerdem können Sie mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dokumente klicken. Jede Möglichkeit fragt nach einer Datei oder verwendet die Datei, auf die Sie geklickt haben:

- **Schreibgeschützt öffnen** öffnet die Datei nur zum Lesen. Sie können sie ansehen, blättern, suchen und kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** drücken, fragt JeffWrite nach einem neuen Namen, sodass die geöffnete Datei unverändert bleibt.
- **Als Kopie öffnen** öffnet ein neues Dokument mit dem Text der Datei. Es hat keine eigene Datei, daher fragt **Speichern**, wo es abgelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** ist für eine beschädigte .docx-Datei gedacht, etwa eine, die beim Kopieren abgebrochen wurde. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .docx-Datei beschädigt. Ein Kopiervorgang wurde mitten drin abgebrochen, oder ein Datenträger hat Fehler gemacht. Eine .docx-Datei ist ein Paket aus kleineren Teilen, etwa dem Text, den Formatvorlagen und den Bildern. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten im Vorgang abbricht, wird bis zu der Stelle übernommen, an der er abbricht. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Wenn die Liste der Teile oder die Verknüpfungen zwischen ihnen fehlen, erstellt JeffWrite sie neu.

Das reparierte Dokument öffnet sich als Kopie mit einem Namen wie „Reparierter Brief“. Eine Meldung teilt Ihnen mit, was bis zu einem Abbruch übernommen und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Kopie unter einem eigenen Namen, und prüfen Sie sie, bevor Sie ihr vertrauen.

## Nicht gespeicherte Arbeit wiederherstellen

JeffWrite speichert alle paar Minuten eine Reservekopie nicht gespeicherter Arbeit als Datei in einem eigenen Ordner. Wird das Programm unerwartet beendet, bietet es diese Kopien beim nächsten Start an. Später sehen Sie sie über **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen**. Sie wählen aus, welche Kopien Sie öffnen möchten, und können jede an einem Ort Ihrer Wahl speichern.

## Mit einer Vorlage beginnen

**Datei > Neu** zeigt das **Leere Dokument** und eine Reihe von Vorlagen, die JeffWrite mitbringt, nach Art gruppiert: Brief, Bericht, Kurzinformation, Manuskript und Newsletter. Jedes Bild zeigt die erste Seite des Dokuments, das Sie erhalten. Eine **Vorlage** ist ein fertig eingerichtetes Dokument mit Formatvorlagen, Seitenrändern und Beispieltext. Klicken Sie auf eine Vorlage, und ersetzen Sie den Beispieltext durch Ihren eigenen. Selbst gespeicherte Vorlagen finden Sie unter **Meine Vorlagen**.

## Dort weitermachen, wo Sie aufgehört haben

JeffWrite merkt sich, wo sich die Einfügemarke befand, als Sie eine Datei gespeichert oder geschlossen haben. Beim nächsten Öffnen der Datei steht die Einfügemarke noch an dieser Stelle. Drücken Sie **Umschalt+F5**, um zu den Stellen zu springen, an denen Sie zuletzt bearbeitet haben. Jeder Druck springt zur davor liegenden Stelle: erst zu den letzten drei Stellen, dann wieder zur ersten.

## Als .odt speichern

Eine .odt-Datei kann nicht alles speichern, was eine .docx-Datei speichern kann. Beim Speichern als .odt zeigt JeffWrite zuerst, was verloren gehen würde, und lässt Sie stattdessen eine .docx-Datei speichern. Siehe [Info](info).

## Drucken

Drücken Sie **Strg+P**, oder klicken Sie auf **Datei > Drucken**. Wählen Sie einen Drucker, die Anzahl der Kopien und die Seiten, sehen Sie sich jede Seite in der Vorschau an, und klicken Sie auf **Drucken**. Siehe [Print](print).

## Ein PDF erstellen

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Wählen Sie, welche Seiten enthalten sein sollen: alle, die aktuelle Seite oder einen Bereich wie 1-3, 5 (p2s3 und s3 funktionieren hier ebenfalls). Wählen Sie dann einen Namen. Das PDF sieht auf jedem Computer gleich aus, weil die Schriftarten in der Datei eingebettet sind. Titel, Autor, Betreff und Stichwörter des Dokuments werden in die PDF-Datei übernommen. Jede Überschrift wird zu einem **Lesezeichen**, einem Eintrag in der Liste, die ein PDF-Reader neben den Seiten anzeigt. Klicken Sie auf ein Lesezeichen, um zu seiner Seite zu springen.

## Eine E-Mail erstellen

Klicken Sie auf **Datei > Exportieren > E-Mail erstellen**, um eine Nachrichtendatei (.eml) zu erstellen. Es wird nichts gesendet. Sie wählen, was in die Nachricht kommt: das Dokument, das Dokument als PDF oder die angezeigten Seiten als Bilder in der Nachricht (die ersten 30 Seiten). Öffnen Sie die Datei in Ihrem E-Mail-Programm, tragen Sie die Empfänger ein, und senden Sie sie.

## Siehe auch

- [Welcome to JeffWrite](start)
- [Print](print)
- [Use JeffWrite with the keyboard](keyboard)
