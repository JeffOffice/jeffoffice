# Öffnen, Speichern, Drucken und Exportieren
<!-- keywords: Webseite, html, als Webseite speichern, gefilterte Webseite, E-Mail, eml, Lesezeichen, PDF-Lesezeichen, Umschalt+F5, letzte Bearbeitung, wo Sie aufgehört haben, integrierte Vorlagen, Katalog, Brief, Bericht, Kurzinformation, Manuskript, Newsletter, öffnen, speichern, speichern unter, docx, doc, dot, odt, OpenDocument, rtf, ältere Dateien, Vorlage, dotx, zuletzt verwendet, anheften, neu, schließen, drucken, PDF, exportieren, wiederherstellen, automatische Wiederherstellung, nicht gespeichert, Absturz, Dateiansicht, Dateiformate, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ott, reparieren, öffnen und reparieren, beschädigt, Kennwort, verschlüsseln, mit Kennwort schützen -->

Klicken Sie oben links auf **Datei**, um die Dateiansicht zu öffnen. Sie hat Seiten für **Neu**, **Öffnen**, **Drucken** und **Exportieren** sowie Befehle für **Speichern** und **Speichern unter**. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Ein neues Dokument beginnen

Drücken Sie **Strg+N**, oder klicken Sie auf **Datei > Neu** und dann auf **Leeres Dokument**. Ein neues Dokument öffnet sich in einem eigenen Fenster. Wenn das Fenster, in dem Sie gerade arbeiten, leer ist und noch nie gespeichert wurde, ersetzt das neue Dokument es.

## Ein Dokument öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen**. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. JeffWrite öffnet diese Dateiarten:

- **.docx** ist ein Dokument.
- **.dotx** ist eine Vorlage. Eine Vorlage öffnet sich als neues Dokument, das auf ihr beruht, sodass die Vorlage selbst unverändert bleibt.
- **.docm** und **.dotm** sind ein Dokument und eine Vorlage, die **Makros** enthalten können, also kleine Programme, die zu einer Datei gehören. JeffWrite führt sie nie aus. Beim Speichern als .docm oder .dotm bleiben sie erhalten. Wenn Sie als .docx oder .odt speichern, werden die Makros weggelassen, und JeffWrite weist Sie darauf hin.
- **.txt** ist Nur-Text, also nur die Wörter. Beim Öffnen liest JeffWrite die Datei sofort, wenn sie reines ASCII oder Unicode ist. Ist das nicht der Fall, fragt JeffWrite, mit welcher **Codierung** sie gelesen werden soll, und zeigt die ersten Zeilen an, damit Sie prüfen können, ob die Buchstaben stimmen. Eine **Codierung** legt fest, wie eine Datei Buchstaben speichert, zum Beispiel UTF-8 oder Windows 1252. Beim Speichern als .txt wählen Sie die Codierung und die Zeilenenden. Die Datei enthält dann nur die Wörter: keine Schriftarten, Bilder oder Tabellenlinien.
- **.html** und **.htm** sind Webseiten. JeffWrite liest die Wörter, Überschriften, fett, kursiv, unterstrichen, Farben, Listen und Tabellen. Aufzählungs- und Nummerierungszeichen von Listen werden als eingegebene Zeichen übernommen. Bilder und Links werden weggelassen. JeffWrite lädt nichts aus dem Internet und ignoriert Skripte. Eine Webseite wird nie überschrieben: **Speichern** fragt nach einem neuen Namen und speichert eine .docx-Datei. Informationen zum Erstellen einer Webseite finden Sie unten unter **Als Webseite speichern**.
- **.doc** und **.dot** sind die älteren Dokument- und Vorlagenformate. JeffWrite liest sie, und was Sie damit tun, speichert JeffWrite als neue .docx-Datei.
- **.odt**, **.ott** und **.fodt** sind OpenDocument-Dateien. JeffWrite liest alle drei. Es speichert .odt-Dateien, und was Sie mit einer .ott- oder .fodt-Datei tun, speichert es als neue .odt-Datei. Siehe [OpenDocument-Dateien öffnen und speichern](open-odt).
- **.rtf** ist Rich Text Format. JeffWrite liest und speichert es. Siehe [RTF-Dateien öffnen](open-rtf).

Auf der Seite **Öffnen** werden auch die Dokumente aufgeführt, die Sie zuletzt geöffnet oder gespeichert haben. Klicken Sie auf eines, um es zu öffnen. Ein Rechtsklick auf ein Dokument heftet es oben in der Liste an oder entfernt es aus der Liste.

Wenn eine Datei Teile enthält, die JeffWrite noch nicht bearbeiten kann, etwa das Layout einer Tabelle oder eine Kopfzeile, bleiben diese erhalten. Siehe [Willkommen bei JeffWrite](start). Wenn ein Teil einer Datei gar nicht gelesen werden kann, teilt JeffWrite mit, was es weggelassen hat. Anschließend wird die Datei nicht überschrieben: **Speichern** fragt nach einem neuen Namen, damit das Original das behält, was die Kopie verloren hätte.

## Ein Dokument speichern

Drücken Sie **Strg+S**, um zu speichern. Beim ersten Mal fragt JeffWrite nach einem Namen. **Speichern unter** (F12) speichert eine Kopie unter einem neuen Namen. Wählen Sie **Vorlage (*.dotx)**, um eine Vorlage zu speichern, **Dokument mit Makros (*.docm)**, um Makros zu behalten, **Nur-Text (*.txt)** für reinen Text oder **Rich Text Format (*.rtf)** für eine Datei, die auch andere Programme lesen können. Eine Datei, die aus einem älteren Dateiformat stammt, wird immer unter einem neuen Namen gespeichert, damit die alte Datei nie überschrieben wird.

## Eine Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**. Wählen Sie einen Namen. JeffWrite schreibt das Dokument in diese Datei, und Sie arbeiten weiter im bisherigen Dokument. Dessen Name und die nicht gespeicherten Änderungen bleiben unverändert. Das ist praktisch, wenn Sie eine Version an jemanden senden und weiterschreiben möchten.

## Eine Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen** und schalten Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen** ein. Bei jedem Speichern kopiert JeffWrite zuerst die alte Datei in eine Datei mit dem Namen „Sicherung von“ plus dem bisherigen Namen, im selben Ordner. Kann die Kopie nicht erstellt werden, fragt JeffWrite, ob ohne Sicherung gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja sagen, und die Datei bleibt unverändert. So können Sie zur zuletzt gespeicherten Version zurückkehren.

## Als Vorlage speichern

Eine Vorlage ist ein Dokument, das Sie als Ausgangspunkt verwenden. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage landet unter **Meine Vorlagen**, und das Dokument, an dem Sie gerade arbeiten, bleibt unverändert. Unter **Speichern unter** können Sie auch **Vorlage (*.dotx)** oder **OpenDocument-Textvorlage (*.ott)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um ein neues Dokument aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen sind unter **Meine Vorlagen** aufgeführt. Klicken Sie auf eine davon. Ein neues Dokument öffnet sich mit dem Text und den Formatvorlagen der Vorlage. Um eine Vorlage aus der Liste zu entfernen, klicken Sie mit der rechten Maustaste darauf und wählen **Aus Meine Vorlagen entfernen**.

## Andere Möglichkeiten, eine Datei zu öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken. Jede dieser Möglichkeiten fragt nach einer Datei oder verwendet die Datei, auf die Sie geklickt haben:

- **Schreibgeschützt öffnen** öffnet die Datei nur zum Lesen. Sie können blättern, suchen und kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“ an. Wenn Sie **Speichern** drücken, fragt JeffWrite nach einem neuen Namen, sodass die geöffnete Datei unverändert bleibt.
- **Als Kopie öffnen** öffnet ein neues Dokument mit dem Text der Datei. Es hat keine eigene Datei, daher fragt **Speichern**, wo es abgelegt werden soll. Die geöffnete Datei wird nie geändert.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** ist für eine beschädigte .docx-Datei gedacht, zum Beispiel eine, die beim Kopieren abgebrochen wurde. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .docx-Datei beschädigt, etwa weil ein Kopiervorgang mitten drin abgebrochen ist oder ein Datenträger Fehler gemacht hat. Eine .docx-Datei ist ein Paket aus kleineren Teilen, etwa dem Text, den Formatvorlagen und den Bildern. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten im Vorgang abbricht, wird bis zu der Stelle übernommen, an der er abbricht. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verknüpfungen zwischen ihnen, erstellt JeffWrite sie neu.

Das reparierte Dokument öffnet sich als Kopie mit einem Namen wie „Repariert Brief“. Eine Meldung teilt Ihnen mit, was bis zu einem Abbruch übernommen wurde und was weggelassen wurde. Die beschädigte Datei wird nicht geändert. Speichern Sie die reparierte Kopie unter einem eigenen Namen und prüfen Sie sie, bevor Sie ihr vertrauen.

## Nicht gespeicherte Arbeit wiederherstellen

JeffWrite speichert alle paar Minuten eine Ersatzkopie der nicht gespeicherten Arbeit als Datei in einem eigenen Ordner. Wird das Programm unerwartet beendet, bietet es diese Kopien beim nächsten Start an. Klicken Sie später auf **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen**, um sie anzuzeigen. Sie können auswählen, welche Kopien Sie öffnen, und jede an einem beliebigen Ort speichern.

## Mit einer Vorlage beginnen

**Datei > Neu** zeigt das **Leere Dokument** und eine Reihe von Vorlagen, die JeffWrite mitbringt, nach Art sortiert: Brief, Bericht, Kurzinformation, Manuskript und Newsletter. Jedes Bild zeigt die erste Seite des Dokuments, das Sie erhalten. Eine **Vorlage** ist ein bereits eingerichtetes Dokument mit Formatvorlagen, Seitenrändern und Beispieltext. Klicken Sie auf eine Vorlage und ersetzen Sie den Beispieltext durch Ihren eigenen. Selbst gespeicherte Vorlagen sind unter **Meine Vorlagen** aufgeführt.

## Dort weitermachen, wo Sie aufgehört haben

JeffWrite merkt sich, wo sich die Einfügemarke befand, als Sie eine Datei gespeichert oder geschlossen haben. Wenn Sie die Datei das nächste Mal öffnen, steht die Einfügemarke an dieser Stelle. Drücken Sie **Umschalt+F5**, um zu den Stellen zu springen, die Sie zuletzt bearbeitet haben. Jeder Tastendruck springt zur davor liegenden Stelle: erst zu den letzten drei Stellen, dann zurück zur ersten.

## Als .odt speichern

Eine .odt-Datei kann nicht alles enthalten, was eine .docx-Datei enthalten kann. Beim Speichern als .odt zeigt JeffWrite zuerst an, was verloren gehen würde, und bietet an, stattdessen eine .docx-Datei zu speichern. Siehe [Informationen](info).

## Drucken **Strg+P**, oder klicken Sie auf **Datei > Drucken**. Wählen Sie einen Drucker, die Anzahl der Exemplare und die Seiten, sehen Sie sich jede Seite in der Vorschau an und klicken Sie auf **Drucken**. Siehe [Drucken](print).

## PDF erstellen

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Wählen Sie, welche Seiten einbezogen werden: alle, die aktuelle Seite oder einen Bereich wie 1-3, 5 (p2s3 und s3 funktionieren hier ebenfalls). Wählen Sie dann einen Namen. Das PDF sieht auf jedem Computer gleich aus, weil die Schriftarten in die Datei eingebettet werden. Titel, Autor, Betreff und Stichwörter des Dokuments werden ins PDF übernommen. Jede Überschrift wird zu einem **Lesezeichen**, einem Eintrag in der Liste, die ein PDF-Programm neben den Seiten anzeigt. Klicken Sie auf ein Lesezeichen, um zu seiner Seite zu springen.

## Als Webseite speichern

Klicken Sie auf **Datei > Exportieren** und wählen Sie eine von zwei Arten von Webseiten. Eine **Webseite** ist eine Datei, die Sie in jedem Webbrowser öffnen können.

- **Als Webseite speichern** erstellt eine einzelne .html-Datei, die Ihre Wörter, deren Aussehen und Ihre Bilder enthält. Sie können diese eine Datei allein versenden. Das Aussehen, das erhalten bleibt, sind Schriftart, Schriftgrad, Farbe, Markierung, fett, kursiv, unterstrichen, Ausrichtung und der Abstand zwischen Absätzen.
- **Als gefilterte Webseite speichern** erstellt eine kleine .html-Datei, die nur die Struktur enthält: Überschriften, fett und kursiv, Listen, Tabellen, Links und Bilder. Sie hat keine Schriftarten, Schriftgrade oder Farben, sodass Sie ihr später ein eigenes Aussehen geben können.

Beide Varianten behalten Überschriften, Aufzählungs- und nummerierte Listen, Tabellen mit verbundenen Zellen, Links und Bilder bei. Ausgeblendeter Text, Wörter, die eine nachverfolgte Änderung gelöscht hat, Kommentare, Kopf- und Fußzeilen sowie Notizen werden weggelassen. Ein Link, der ein Programm starten oder ein Skript ausführen würde, wird als reiner Text gespeichert.

## PDF mit einem Kennwort schützen

Klicken Sie im Feld **Als PDF exportieren** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer das PDF danach öffnet, muss das Kennwort eingeben. Das Kennwort schützt die Datei tatsächlich: Die Seiten werden mit starker Verschlüsselung (AES-256) unlesbar gemacht, und ohne Kennwort kann niemand sie lesen.

Sie können außerdem **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Programme halten sich daran, aber wer sie umgehen will, wird dadurch nicht gehindert. Verlassen Sie sich deshalb nicht darauf, um eine Datei vertraulich zu halten.

Ein vergessenes Kennwort lässt sich nicht wiederherstellen. Bewahren Sie es daher sicher auf. Das Kennwort gilt nur für dieses eine PDF und wird nie gespeichert. Solange ein Kennwort gesetzt ist, ist **PDF/A** im Feld deaktiviert, weil eine PDF/A-Datei kein Kennwort haben kann.

## PDF/A erstellen

Setzen Sie im Feld **Als PDF exportieren** einen Haken bei **PDF/A zur langfristigen Aufbewahrung**. Eine PDF/A-Datei enthält alle Schriftarten, die sie verwendet, und folgt einem Standard, der für die Aufbewahrung von Dateien über viele Jahre gedacht ist. Sie ist noch nicht für Bildschirmleseprogramme gekennzeichnet, und sie kann kein Kennwort haben.

## E-Mail erstellen

Klicken Sie auf **Datei > Exportieren > E-Mail erstellen**, um eine Nachrichtendatei (.eml) zu erstellen. Es wird nichts gesendet. Sie wählen, was in die Nachricht kommt: das Dokument, das Dokument als PDF oder die angezeigten Seiten als Bilder in der Nachricht (die ersten 30 Seiten). Öffnen Sie die Datei in Ihrem E-Mail-Programm, tragen Sie den Empfänger ein und senden Sie sie. Bei einem PDF können Sie im Feld auf **Mit Kennwort verschlüsseln…** klicken. Teilen Sie der Person das Kennwort auf einem anderen Weg mit als in der Nachricht.

## Siehe auch

- [Willkommen bei JeffWrite](start)
- [Drucken](print)
- [JeffWrite mit der Tastatur bedienen](keyboard)
