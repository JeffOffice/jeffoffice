# Öffnen, Speichern, Drucken und Exportieren
<!-- keywords: Webseite, html, als Webseite speichern, gefilterte Webseite, E-Mail, eml, Textmarken, PDF-Lesezeichen, Umschalt+F5, letzte Bearbeitung, wo Sie aufgehört haben, integrierte Vorlagen, Katalog, Brief, Bericht, Kurzinformation, Manuskript, Newsletter, öffnen, speichern, speichern unter, docx, doc, dot, odt, OpenDocument, rtf, ältere Dateien, Vorlage, dotx, zuletzt verwendet, anheften, neu, schließen, drucken, PDF, exportieren, wiederherstellen, automatische Wiederherstellung, nicht gespeichert, Absturz, Dateiansicht, Dateiformate, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ott, reparieren, Öffnen und reparieren, beschädigt -->

Klicken Sie oben links auf **Datei**, um die Dateiansicht zu öffnen. Sie hat Seiten für **Neu**, **Öffnen**, **Drucken** und **Exportieren** sowie Befehle für **Speichern** und **Speichern unter**. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Ein neues Dokument beginnen

Drücken Sie **Strg+N**, oder klicken Sie auf **Datei > Neu** und dann auf **Leeres Dokument**. Ein neues Dokument öffnet sich in einem eigenen Fenster. Ist das aktuelle Fenster leer und noch nie gespeichert worden, ersetzt das neue Dokument dessen Inhalt.

## Ein Dokument öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen**. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. JeffWrite öffnet diese Dateiarten:

- **.docx** ist ein Dokument.
- **.dotx** ist eine Vorlage. Eine Vorlage öffnet sich als neues Dokument, das auf ihr basiert, sodass die Vorlage unverändert bleibt.
- **.docm** und **.dotm** sind ein Dokument und eine Vorlage, die **Makros** enthalten können, kleine Programme, die zu einer Datei gehören. JeffWrite führt sie nie aus. Beim Speichern als .docm oder .dotm bleiben sie erhalten. Speichern Sie als .docx oder .odt, werden die Makros weggelassen, und JeffWrite weist Sie darauf hin.
- **.txt** ist reiner Text, nur die Wörter. Beim Öffnen liest JeffWrite die Datei sofort, wenn sie reines ASCII oder Unicode ist. Ist das nicht der Fall, fragt JeffWrite nach der Codierung und zeigt die ersten Zeilen, damit Sie sehen können, ob die Buchstaben stimmen. Eine **Codierung** legt fest, wie eine Datei Buchstaben speichert, zum Beispiel UTF-8 oder Windows-1252. Beim Speichern als .txt wählen Sie die Codierung und die Zeilenenden. Die Datei behält nur die Wörter: keine Schriftarten, Bilder oder Tabellenlinien.
- **.html** und **.htm** sind Webseiten. JeffWrite liest die Wörter, Überschriften, fett, kursiv, unterstrichen, Farben, Listen und Tabellen. Aufzählungszeichen und Nummern von Listen werden als eingegebene Zeichen übernommen, Bilder und Links werden weggelassen. JeffWrite lädt nie etwas aus dem Internet und ignoriert Skripte. Eine Webseite wird nie überschrieben: **Speichern** fragt nach einem neuen Namen und speichert eine .docx-Datei. Eine Webseite erstellen Sie unter **Als Webseite speichern** weiter unten.
- **.doc** und **.dot** sind die älteren Dokument- und Vorlagenformate. JeffWrite liest sie, und Ihre Änderungen speichert es als neue .docx-Datei.
- **.odt**, **.ott** und **.fodt** sind OpenDocument-Dateien. JeffWrite liest alle drei. Es speichert .odt-Dateien, und Ihre Änderungen an einer .ott- oder .fodt-Datei speichert es als neue .odt-Datei. Siehe [Open-Document-Dateien öffnen und speichern](open-odt).
- **.rtf** ist Rich Text Format. JeffWrite liest und speichert es. Siehe [RTF-Dateien öffnen](open-rtf).

Auf der Seite **Öffnen** stehen auch die Dokumente, die Sie zuletzt geöffnet oder gespeichert haben. Klicken Sie auf eines, um es zu öffnen. Mit der rechten Maustaste können Sie ein Dokument oben in der Liste anheften oder es aus der Liste entfernen.

Wenn eine Datei Teile enthält, die JeffWrite noch nicht bearbeiten kann, etwa das Layout einer Tabelle oder eine Kopfzeile, bleiben diese erhalten. Siehe [Willkommen bei JeffWrite](start). Kann ein Teil einer Datei gar nicht gelesen werden, teilt JeffWrite mit, was es weggelassen hat. Dann wird diese Datei nicht überschrieben: **Speichern** fragt nach einem neuen Namen, damit das Original erhalten bleibt, auch wenn die Kopie etwas verloren hat.

## Ein Dokument speichern

Drücken Sie **Strg+S**, um zu speichern. Beim ersten Mal fragt JeffWrite nach einem Namen. **Speichern unter** (F12) speichert das Dokument unter einem neuen Namen. Wählen Sie **Vorlage (*.dotx)**, um eine Vorlage zu speichern, **Dokument mit Makros (*.docm)**, um Makros zu behalten, **Nur-Text (*.txt)** für nur die Wörter oder **Rich Text Format (*.rtf)** für eine Datei, die andere Programme lesen können. Eine Datei, die aus einem älteren Dateiformat stammt, wird immer unter einem neuen Namen gespeichert, damit die alte Datei nie überschrieben wird.

## Eine Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**. Wählen Sie einen Namen. JeffWrite schreibt das Dokument in diese Datei, und Sie arbeiten im bisherigen Dokument weiter. Name und nicht gespeicherte Änderungen bleiben unverändert. Das ist praktisch, wenn Sie eine Fassung verschicken und weiterschreiben möchten.

## Eine Sicherungskopie behalten

Klicken Sie auf **Datei > Optionen** und schalten Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen** ein. Bei jedem Speichern kopiert JeffWrite zuerst die alte Datei in eine Datei mit dem Namen „Sicherung von“ plus dem bisherigen Namen, im selben Ordner. Kann die Kopie nicht erstellt werden, fragt JeffWrite, ob ohne Kopie gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja sagen, und die Datei bleibt unverändert. So können Sie zur zuletzt gespeicherten Fassung zurückkehren.

## Als Vorlage speichern

Eine Vorlage ist ein Dokument, das Sie als Ausgangspunkt verwenden. Klicken Sie auf **Datei > Als Vorlage speichern**, und geben Sie einen Namen ein. Die Vorlage landet unter **Meine Vorlagen**, und das Dokument, an dem Sie gerade arbeiten, bleibt unverändert. Unter **Speichern unter** können Sie auch **Vorlage (*.dotx)** oder **OpenDocument-Textvorlage (*.ott)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um ein neues Dokument aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen werden unter **Meine Vorlagen** aufgelistet. Klicken Sie auf eine davon. Ein neues Dokument öffnet sich mit dem Text und den Formatvorlagen der Vorlage. Um eine Vorlage aus der Liste zu entfernen, klicken Sie mit der rechten Maustaste darauf und wählen **Aus Meine Vorlagen entfernen**.

## Weitere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken. Jede Möglichkeit fragt nach einer Datei oder verwendet die, auf die Sie geklickt haben:

- **Schreibgeschützt öffnen** öffnet die Datei nur zum Lesen. Sie können sie ansehen, scrollen, suchen und kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie auf **Speichern** klicken, fragt JeffWrite nach einem neuen Namen, sodass die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet ein neues Dokument mit dem Text der Datei. Es hat keine eigene Datei, daher fragt **Speichern**, wo es abgelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .docx-Datei, etwa eine, die beim Kopieren abgebrochen wurde. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .docx-Datei beschädigt. Ein Kopiervorgang wurde halb abgebrochen, oder eine Festplatte hat Fehler gemacht. Eine .docx-Datei besteht aus kleineren Teilen, etwa dem Text, den Formatvorlagen und den Bildern. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten im Vorgang abbrach, wird bis zu der Stelle übernommen, an der er abbrach. Ein Teil, der nicht gelesen werden kann, wird weggelassen. Sind die Liste der Teile oder die Verknüpfungen zwischen ihnen verloren, erstellt JeffWrite sie neu.

Das reparierte Dokument öffnet sich als Kopie mit einem Namen wie „Repariertes Anschreiben“. Eine Meldung teilt Ihnen mit, was bis zu einem Abbruch übernommen und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Kopie unter einem eigenen Namen, und prüfen Sie sie, bevor Sie ihr vertrauen.

## Nicht gespeicherte Arbeit wiederherstellen

JeffWrite speichert alle paar Minuten eine Ersatzkopie nicht gespeicherter Arbeit als Datei in einem eigenen Ordner. Wenn das Programm unerwartet beendet wird, bietet es diese Kopien beim nächsten Start an. Klicken Sie auf **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen**, um sie später anzusehen. Sie können auswählen, welche Kopie Sie öffnen, und jede dort speichern, wo Sie möchten.

## Mit einer Vorlage beginnen

**Datei > Neu** zeigt das **Leere Dokument** und eine Auswahl an Vorlagen, die mit JeffWrite geliefert werden, gruppiert nach Art: Brief, Bericht, Kurzinformation, Manuskript und Newsletter. Jedes Bild zeigt die erste Seite des Dokuments, das Sie erhalten. Eine **Vorlage** ist ein fertig eingerichtetes Dokument mit Formatvorlagen, Seitenrändern und Beispieltext. Klicken Sie auf eine Vorlage und ersetzen Sie den Beispieltext durch Ihren eigenen. Selbst gespeicherte Vorlagen werden unter **Meine Vorlagen** aufgelistet.

## Dort weitermachen, wo Sie aufgehört haben

JeffWrite merkt sich, wo sich die Einfügemarke befand, als Sie eine Datei gespeichert oder geschlossen haben. Beim nächsten Öffnen der Datei steht die Einfügemarke dort. Drücken Sie **Umschalt+F5**, um zu den Stellen zu springen, an denen Sie zuletzt bearbeitet haben. Jeder Druck springt zur jeweils vorherigen Stelle: zuerst zu den letzten drei Stellen, dann zurück zur ersten.

## Als .odt speichern

Eine .odt-Datei kann nicht alles enthalten, was eine .docx-Datei kann. Beim Speichern als .odt zeigt JeffWrite zuerst an, was verloren gehen würde, und bietet an, stattdessen eine .docx-Datei zu speichern. Siehe [Info](info).

## Drucken

Drücken Sie **Strg+P**, oder klicken Sie auf **Datei > Drucken**. Wählen Sie einen Drucker, die Anzahl der Kopien und die Seiten, sehen Sie sich jede Seite in der Vorschau an und klicken Sie auf **Drucken**. Siehe [Drucken](print).

## Eine PDF-Datei erstellen

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Wählen Sie, welche Seiten aufgenommen werden sollen: alle, die aktuelle Seite oder ein Bereich wie 1-3, 5 (p2s3 und s3 funktionieren hier ebenfalls). Wählen Sie dann einen Namen. Die PDF-Datei sieht auf jedem Computer gleich aus, weil die Schriftarten in ihr eingebettet sind. Titel, Autor, Betreff und Stichwörter des Dokuments werden in die PDF-Datei übernommen. Jede Überschrift wird zu einem **Lesezeichen**, einem Eintrag in der Liste, die ein PDF-Reader neben den Seiten anzeigt. Klicken Sie auf ein Lesezeichen, um zu dessen Seite zu springen.

## Als Webseite speichern

Klicken Sie auf **Datei > Exportieren** und wählen Sie eine von zwei Arten von Webseiten. Eine **Webseite** ist eine Datei, die Sie in jedem Webbrowser öffnen können.

- **Als Webseite speichern** erstellt eine einzige .html-Datei, die Ihre Wörter, deren Aussehen und Ihre Bilder enthält. Diese eine Datei können Sie allein versenden. Das Aussehen, das sie beibehält, sind Schriftart, Schriftgrad, Farbe, Hervorhebung, fett, kursiv, unterstrichen, Ausrichtung und der Abstand um die Absätze.
- **Als gefilterte Webseite speichern** erstellt eine kleine .html-Datei, die nur die Struktur enthält: Überschriften, fett und kursiv, Listen, Tabellen, Links und Bilder. Sie hat keine Schriftarten, Schriftgrade oder Farben, sodass Sie ihr später Ihr eigenes Aussehen geben können.

Beide behalten Überschriften, Aufzählungs- und nummerierte Listen, Tabellen mit verbundenen Zellen, Links und Bilder bei. Ausgeblendeter Text, Wörter, die eine nachverfolgte Änderung gelöscht hat, Kommentare, Kopf- und Fußzeilen sowie Fußnoten und Endnoten werden weggelassen. Ein Link, der ein Programm starten oder ein Skript ausführen würde, wird als reiner Text gespeichert.

## Eine PDF/A-Datei erstellen

Aktivieren Sie im Feld **Als PDF exportieren** das Kontrollkästchen **PDF/A, zur langfristigen Aufbewahrung**. Eine PDF/A-Datei enthält alle Schriftarten, die sie verwendet, und folgt einer Norm für die Aufbewahrung von Dateien über viele Jahre. Sie ist noch nicht für Bildschirmlesegeräte ausgezeichnet und hat kein Kennwort.

## Eine E-Mail erstellen

Klicken Sie auf **Datei > Exportieren > E-Mail erstellen**, um eine Nachrichtendatei (.eml) zu erstellen. Es wird nichts gesendet. Sie legen fest, was in die Nachricht kommt: das Dokument, das Dokument als PDF oder die Seiten als Bilder in der Nachricht (die ersten 30 Seiten). Öffnen Sie die Datei in Ihrem E-Mail-Programm, geben Sie den Empfänger an und senden Sie sie.

## Siehe auch

- [Willkommen bei JeffWrite](start)
- [Drucken](print)
- [JeffWrite mit der Tastatur verwenden](keyboard)
