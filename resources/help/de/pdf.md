# PDF-Dateien und Druckereien
<!-- keywords: pdf, pdf erstellen, als pdf speichern, pdf exportieren, pdf/x, pdf/a, archivieren, archiv, druckerei, professioneller druck, cmyk, anschnitt, schnittmarken, farbprofil, volltonfarbe, überdrucken, kennwort, verschlüsseln, mit kennwort schützen, passwort -->

Eine **PDF**-Datei sieht auf jedem Computer gleich aus. Verwenden Sie sie, um eine Publikation per E-Mail zu versenden, online zu veröffentlichen oder an eine Druckerei zu schicken.

## PDF erstellen

1. Klicken Sie auf **Datei > Exportieren** und dann auf **PDF erstellen**.
2. Wählen Sie unter **Optimieren für** eine Qualität. **dpi** steht für Punkte pro Zoll (dots per inch). Mehr Punkte ergeben schärfere Bilder und größere Dateien.
3. Wählen Sie unter **Seiten** die Option **Alle Seiten**, **Aktuelle Seite** oder **Seiten von** einer Seite **bis** einer anderen.
4. Klicken Sie auf **PDF speichern…**, geben Sie einen Namen ein, und klicken Sie auf **Speichern**.

| Auswahl | Bilder | Geeignet für |
|---|---|---|
| Minimale Größe | 96 dpi | Anzeige auf dem Bildschirm |
| Standard | 150 dpi | Weitergabe online |
| Hohe Qualität beim Drucken | 300 dpi | Desktopdrucker |
| Professioneller Druck | Volle Auflösung | Eine Druckerei. Fügt Schnittmarken, Anschnittmarken, Passermarken, Farbbalken und Auftragsinformationen hinzu. |

Weitere Optionen:

- **Dokumenteigenschaften einschließen** übernimmt Titel, Autor, Betreff und Stichwörter in die PDF-Datei.
- **Dokumentstruktur-Tags für Barrierefreiheit** (standardmäßig aktiviert) erstellt eine *getaggte* PDF-Datei. Die Tags teilen einem Bildschirmleseprogramm mit, was eine Überschrift, ein Absatz, ein Bild oder eine Tabelle ist, und in welcher Reihenfolge es vorgelesen werden soll: in der Reihenfolge der Objekte auf jeder Seite, die Sie in der Liste **Lesereihenfolge** der Barrierefreiheitsprüfung sehen und ändern können. Ein Bild wird mit seinem Alternativtext vorgelesen. Eine PDF/X-Datei enthält keine Tags.
- **Textmarken erstellen** (standardmäßig aktiviert) legt für jede Seite eine Textmarke in der PDF-Datei an. Sie erhält den Namen der Seite oder „Seite 3“, wenn die Seite keinen Namen hat. Ein PDF-Reader zeigt sie neben den Seiten an.
- **PDF nach dem Speichern öffnen** zeigt die PDF-Datei sofort in Ihrem Anzeigeprogramm an.
- **Broschürenbögen** erscheint bei Broschüren. Es werden je zwei Seiten in Falzreihenfolge auf eine Seite gesetzt. Deaktivieren Sie die Option, um eine Seite pro PDF-Seite zu erhalten.

## PDF/A für die Archivierung

**PDF/A** ist eine PDF-Datei, die jahrelang lesbar bleiben soll. Sie enthält alle Schriftarten und verwendet Standardfarben, ohne transparente Effekte. Aktivieren Sie **PDF/A für die Langzeitarchivierung**, oder klicken Sie auf **Datei > Exportieren** und dann auf **PDF/A zur Archivierung erstellen**.

## PDF/X für eine Druckerei

**PDF/X** ist eine strengere PDF-Variante, die Druckereien verlangen. Farben werden als Tintenmengen angegeben, Schriftarten sind eingebettet, und Beschnitt und Anschnitt sind markiert. Der **Anschnitt** ist zusätzliches Bild oder Farbe, das bzw. die über den Seitenrand hinausreicht, damit beim Beschneiden kein weißer Rand entsteht.

1. Klicken Sie auf **Datei > Exportieren** und dann auf **PDF/X für eine Druckerei erstellen**. Oder aktivieren Sie im Fenster **PDF erstellen** die Option **PDF/X für eine Druckerei**. Sie lässt sich nicht mit PDF/A kombinieren.
2. Wählen Sie unter **Druckbedingung** die Bedingung, die Ihre Druckerei angibt.
3. Wenn die Druckerei Schnittmarken und einen Anschnitt verlangt, wählen Sie zusätzlich unter **Optimieren für** die Option **Professioneller Druck**.
4. Klicken Sie auf **PDF speichern…**.

Die Bedingungen sind:

- **PDF/X-1a** für US-Rollenoffset (SWOP) oder europäischen Offsetdruck (FOGRA39). Transparente Effekte werden reduziert.
- **PDF/X-4** für hochwertiges gestrichenes Papier (FOGRA51, PSO Coated v3) oder mit dem eigenen Farbprofil Ihrer Druckerei (eine .icc-Datei, die Sie auswählen). Transparente Effekte bleiben erhalten.

Das Farbprofil PSO Coated v3 ist nicht in JeffPub enthalten. Beim ersten Gebrauch fragt JeffPub, bevor es das Profil einmalig von der European Color Initiative herunterlädt.

Bevor die Datei erstellt wird, prüft JeffPub, ob Bilder mit weniger als 150 ppi (Pixel pro Zoll) in ihrer gedruckten Größe vorhanden sind und ob Schriftarten auf diesem Computer fehlen. Klicken Sie auf **Trotzdem PDF erstellen**, **Entwurfsprüfung öffnen** oder **Abbrechen**.

## PDF mit einem Kennwort schützen

Klicken Sie im Fenster **PDF erstellen** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer die PDF-Datei öffnet, muss dann das Kennwort eingeben. Das Kennwort schützt die Datei tatsächlich: Die Seiten werden mit starker Verschlüsselung (AES-256) verwürfelt, und niemand kann sie ohne Kennwort lesen.

Sie können außerdem **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Reader halten sich daran, aber wer sie umgehen will, wird dadurch nicht gehindert. Verlassen Sie sich daher nicht darauf, eine Datei damit privat zu halten.

Ein vergessenes Kennwort lässt sich nicht wiederherstellen. Bewahren Sie es daher sicher auf. Das Kennwort gilt nur für diese eine PDF-Datei und wird nicht gespeichert. Eine PDF-Datei mit Kennwort kann weder PDF/A noch PDF/X sein, daher sind beide Optionen deaktiviert, solange ein Kennwort gesetzt ist. Unter **Datei > Freigeben** setzt **Mit Kennwort verschlüsseln…** oberhalb von **Als PDF senden** ein Kennwort auf die PDF-Datei in einer E-Mail.

Wenn Sie eine PDF-Datei mit Kennwort als Bild einfügen, fragt JeffPub nach dem Kennwort. Ist es falsch, fragt JeffPub erneut; klicken Sie auf **Abbrechen**, um abzubrechen. Die Seite in Ihrer Publikation ist eine Kopie und benötigt kein Kennwort. Wenn der Eigentümer der PDF-Datei verlangt, dass sie nicht kopiert wird, fügt JeffPub sie trotzdem ein und zeigt einen entsprechenden Hinweis an. Diese Bitte ist keine Sperre.

Ein Bild, das mit einer PDF-Datei mit Kennwort verknüpft ist, wird beim Öffnen der Publikation als **Nicht aktualisiert** angezeigt. JeffPub fragt nie von sich aus nach dem Kennwort. Klicken Sie auf **Verknüpfung aktualisieren**, um danach gefragt zu werden.

## Farben für eine Druckerei einrichten

Klicken Sie auf **Datei > Info** und dann auf die Karte **Einstellungen für professionellen Druck**:

- **Farbmodell** ist **RGB**, **Einzelfarbe (Volltonfarbe)**, **Volltonfarben**, **Prozessfarben (CMYK)** oder **Prozess- und Volltonfarben**. Prozessfarben sind die Druckfarben Cyan, Magenta, Gelb und Schwarz. Eine Volltonfarbe ist eine Sonderfarbe, die die Druckerei mischt. Fügen Sie jede unter **Volltonfarben** hinzu.
- **Schwarz in Dateien für eine Druckerei überdrucken (PDF/X)** lässt Schwarz über die darunterliegenden Farben drucken, damit eine verschobene Platte keinen weißen Rand hinterlässt. Sie legen fest, was betroffen ist: Text unter einer bestimmten Größe, Linien oder Füllungen.

## Siehe auch

- [Publikation drucken](printing)
- [Als Bild, Webseite oder E-Book speichern](export)
- [Entwurf prüfen](design-checker)
