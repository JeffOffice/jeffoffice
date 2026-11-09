# PDF-Dateien und Druckereien
<!-- keywords: pdf, pdf erstellen, als pdf speichern, pdf exportieren, pdf/x, pdf/a, archivieren, archivierung, druckerei, professioneller druck, druckdaten, cmyk, anschnitt, beschnitt, schnittmarken, farbprofil, volltonfarbe, überdrucken, kennwort, passwort, verschlüsseln, mit kennwort schützen, mit passwort schützen -->

Eine **PDF** ist eine Datei, die auf jedem Computer gleich aussieht. Verwenden Sie sie, um eine Publikation per E-Mail zu versenden, online zu veröffentlichen oder an eine Druckerei zu schicken.

## PDF erstellen

1. Klicken Sie auf **Datei > Exportieren** und dann auf **PDF erstellen**.
2. Wählen Sie unter **Optimieren für** eine Qualität. **dpi** bedeutet Punkte pro Zoll. Mehr Punkte ergeben schärfere Bilder und größere Dateien.
3. Wählen Sie unter **Seiten** die Option **Alle Seiten**, **Aktuelle Seite** oder **Seiten von** einer Seite **bis** einer anderen.
4. Klicken Sie auf **PDF speichern…**, geben Sie einen Namen ein und klicken Sie auf **Speichern**.

| Auswahl | Bilder | Geeignet für |
|---|---|---|
| Minimale Größe | 96 dpi | Anzeige auf dem Bildschirm |
| Standard | 150 dpi | Weitergabe online |
| Hochwertiger Druck | 300 dpi | Desktopdrucker |
| Professioneller Druck | Volle Auflösung | Eine Druckerei. Fügt Schnittmarken, Anschnittmarken, Passermarken, Farbbalken und Auftragsinformationen hinzu. |

Weitere Optionen:

- **Dokumenteigenschaften einschließen** übernimmt Titel, Autor, Betreff und Stichwörter in die PDF-Datei.
- **PDF nach dem Speichern öffnen** zeigt die PDF-Datei sofort in Ihrem Anzeigeprogramm an.
- **Broschürenbögen** erscheint bei Broschüren. Dabei werden auf jeder Seite zwei Seiten in der Falzreihenfolge angeordnet. Deaktivieren Sie die Option, um eine Seite pro PDF-Seite zu erhalten.

## PDF/A zur Aufbewahrung

**PDF/A** ist eine PDF-Datei, die über Jahre lesbar bleiben soll. Sie enthält alle Schriftarten und verwendet Standardfarben, ohne transparente Effekte. Aktivieren Sie **PDF/A für die Langzeitarchivierung** oder klicken Sie auf **Datei > Exportieren** und dann auf **PDF/A zur Archivierung erstellen**.

## PDF/X für eine Druckerei

**PDF/X** ist eine strengere PDF-Variante, die Druckereien verlangen. Farben werden als Tintenmengen angegeben, Schriftarten sind eingebettet, und das Endformat sowie der Anschnitt sind markiert. Der **Anschnitt** ist zusätzliches Bild oder Farbe, das über den Seitenrand hinausreicht, damit beim Beschneiden kein weißer Rand bleibt.

1. Klicken Sie auf **Datei > Exportieren** und dann auf **PDF/X für eine Druckerei erstellen**. Alternativ aktivieren Sie im Fenster **PDF erstellen** die Option **PDF/X für eine Druckerei**. Sie können sie nicht zusammen mit PDF/A verwenden.
2. Wählen Sie unter **Druckbedingung** die Bedingung, die Ihre Druckerei nennt.
3. Wenn die Druckerei Schnittmarken und einen Anschnitt verlangt, wählen Sie außerdem unter **Optimieren für** die Option **Professioneller Druck**.
4. Klicken Sie auf **PDF speichern…**.

Die Bedingungen sind:

- **PDF/X-1a** für US-Rollenoffset (SWOP) oder europäischen Offsetdruck (FOGRA39). Transparente Effekte werden reduziert.
- **PDF/X-4** für hochwertiges gestrichenes Papier (FOGRA51, PSO Coated v3) oder mit dem Farbprofil Ihrer Druckerei (eine von Ihnen gewählte .icc-Datei). Transparente Effekte bleiben erhalten.

Das Farbprofil PSO Coated v3 ist nicht in JeffPub enthalten. Beim ersten Gebrauch fragt JeffPub, ob es das Profil einmalig von der European Color Initiative herunterladen darf.

Bevor JeffPub die Datei erstellt, prüft es, ob Bilder in ihrer gedruckten Größe unter 150 ppi (Pixel pro Zoll) liegen und ob Schriftarten auf diesem Computer fehlen. Klicken Sie auf **Trotzdem PDF erstellen**, **Entwurfsprüfung öffnen** oder **Abbrechen**.

## PDF mit einem Kennwort schützen

Klicken Sie im Fenster **PDF erstellen** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer die PDF-Datei öffnet, muss dann das Kennwort eingeben. Das Kennwort schützt die Datei tatsächlich: Die Seiten werden mit starker Verschlüsselung (AES-256) verschlüsselt, und ohne Kennwort kann niemand sie lesen.

Sie können außerdem **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Viewer halten sich daran, aber sie hindern niemanden, der sie umgehen will. Verlassen Sie sich deshalb nicht darauf, eine Datei vertraulich zu halten.

Ein vergessenes Kennwort kann nicht wiederhergestellt werden. Bewahren Sie es daher sicher auf. Das Kennwort gilt nur für diese eine PDF-Datei und wird nie gespeichert. Eine PDF-Datei mit Kennwort kann nicht als PDF/A oder PDF/X gespeichert werden; solange ein Kennwort gesetzt ist, sind diese beiden Optionen deshalb deaktiviert. Unter **Datei > Teilen** können Sie über **Mit Kennwort verschlüsseln…** oberhalb von **Als PDF senden** ein Kennwort für die PDF-Datei in einer E-Mail festlegen.

Wenn Sie eine PDF-Datei mit Kennwort als Bild einfügen, fragt JeffPub nach dem Kennwort. Ist es falsch, fragt JeffPub erneut; mit **Abbrechen** beenden Sie den Vorgang. Die Seite in Ihrer Publikation ist eine Kopie und benötigt kein Kennwort. Wenn der Eigentümer der PDF-Datei verlangt, dass sie nicht kopiert wird, fügt JeffPub sie trotzdem ein und zeigt einen entsprechenden Hinweis; diese Bitte ist keine Sperre.

Ein Bild, das mit einer kennwortgeschützten PDF-Datei verknüpft ist, wird beim Öffnen der Publikation als **Nicht aktualisiert** angezeigt. JeffPub fragt nie von selbst nach dem Kennwort. Klicken Sie auf **Verknüpfung aktualisieren**, damit nach dem Kennwort gefragt wird.

## Farben für eine Druckerei einrichten

Klicken Sie auf **Datei > Info** und dann auf die Karte **Einstellungen für professionellen Druck**:

- **Farbmodell** ist **RGB**, **Einzelfarbe (Volltonfarbe)**, **Volltonfarben**, **Prozessfarben (CMYK)** oder **Prozess- und Volltonfarben**. Prozessfarben sind die Tinten Cyan, Magenta, Gelb und Schwarz. Eine Volltonfarbe ist eine Sonderfarbe, die die Druckerei mischt. Fügen Sie jede Volltonfarbe unter **Volltonfarben** hinzu.
- **Schwarz in Dateien für eine Druckerei überdrucken (PDF/X)** lässt Schwarz über die darunterliegenden Farben drucken, sodass eine verschobene Druckplatte keinen weißen Rand hinterlässt. Sie bestimmen, was betroffen ist: Text unter einer bestimmten Größe, Linien oder Füllungen.

## Siehe auch

- [Publikation drucken](printing)
- [Als Bild, Webseite oder E-Book speichern](export)
- [Entwurf prüfen](design-checker)
