# PDF-Dateien und Druckereien
<!-- keywords: pdf, pdf erstellen, als pdf speichern, pdf exportieren, pdf/x, pdf/a, archivieren, archivierung, druckerei, professioneller druck, cmyk, anschnitt, beschnitt, schnittmarken, farbprofil, icc, volltonfarbe, sonderfarbe, überdrucken -->

Eine **PDF**-Datei sieht auf jedem Computer gleich aus. Verwenden Sie eine PDF-Datei, um eine Publikation per E-Mail zu senden, online zu veröffentlichen oder an eine Druckerei zu schicken.

## PDF erstellen

1. Klicken Sie auf **Datei > Exportieren** und dann auf **PDF erstellen**.
2. Wählen Sie unter **Optimieren für** eine Qualität aus. **dpi** steht für „dots per inch“ (Punkte pro Zoll). Mehr Punkte ergeben schärfere Bilder und größere Dateien.
3. Wählen Sie unter **Seiten** die Option **Alle Seiten**, **Aktuelle Seite** oder **Seiten von** einer Seite **bis** zu einer anderen.
4. Klicken Sie auf **PDF speichern…**, geben Sie einen Namen ein, und klicken Sie auf **Speichern**.

| Option | Bilder | Geeignet für |
|---|---|---|
| Minimale Größe | 96 dpi | Anzeige am Bildschirm |
| Standard | 150 dpi | Onlineveröffentlichung |
| Hohe Druckqualität | 300 dpi | Desktopdrucker |
| Professioneller Druck | Volle Auflösung | Eine Druckerei. Fügt Schnittmarken, Anschnittmarken, Passermarken, Farbbalken und Auftragsinformationen hinzu. |

Weitere Optionen:

- **Dokumenteigenschaften einschließen** fügt Titel, Autor, Betreff und Stichwörter in die PDF-Datei ein.
- **PDF nach dem Speichern öffnen** zeigt die PDF-Datei sofort in Ihrem PDF-Viewer an.
- **Broschürenbogen** wird bei Broschüren angezeigt. Damit werden auf jeder Seite des Bogens zwei Seiten in Falzreihenfolge angeordnet. Deaktivieren Sie die Option, um eine Seite pro PDF-Seite zu erhalten.

## PDF/A für die Archivierung

**PDF/A** ist ein PDF-Format, das für eine jahrelange Aufbewahrung gedacht ist. Es enthält alle Schriftarten und verwendet Standardfarben ohne transparente Effekte. Aktivieren Sie **PDF/A für die Langzeitarchivierung**, oder klicken Sie auf **Datei > Exportieren** und dann auf **PDF/A zur Archivierung erstellen**.

## PDF/X für eine Druckerei

**PDF/X** ist ein strengeres PDF-Format, das Druckereien verlangen. Farben werden als Farbmengen angegeben, Schriftarten sind eingebettet, und Endformat und Anschnitt sind gekennzeichnet. Der **Anschnitt** ist zusätzliches Bild oder zusätzliche Farbe, die über den Seitenrand hinausreicht, damit nach dem Beschneiden kein weißer Rand bleibt.

1. Klicken Sie auf **Datei > Exportieren** und dann auf **PDF/X für eine Druckerei erstellen**. Oder aktivieren Sie im Fenster **PDF erstellen** die Option **PDF/X für eine Druckerei**. Diese Option lässt sich nicht mit PDF/A kombinieren.
2. Wählen Sie unter **Druckbedingung** die Bedingung aus, die Ihre Druckerei angibt.
3. Wenn die Druckerei Schnittmarken und einen Anschnitt wünscht, wählen Sie unter **Optimieren für** außerdem **Professioneller Druck** aus.
4. Klicken Sie auf **PDF speichern…**.

Die Druckbedingungen sind:

- **PDF/X-1a** für US-Rollenoffset (SWOP) oder europäischen Offsetdruck (FOGRA39). Transparente Effekte werden dabei reduziert.
- **PDF/X-4** für hochwertiges gestrichenes Papier (FOGRA51, PSO Coated v3) oder mit dem eigenen Farbprofil Ihrer Druckerei (einer ICC-Datei, die Sie auswählen). Transparente Effekte bleiben erhalten.

Das Farbprofil PSO Coated v3 ist nicht im Lieferumfang von JeffPub enthalten. Bei der ersten Verwendung fragt JeffPub nach, bevor das Profil einmalig von der European Color Initiative heruntergeladen wird.

Bevor die Datei erstellt wird, prüft JeffPub, ob Bilder in ihrer Druckgröße weniger als 150 ppi (Pixel pro Zoll) haben und ob Schriftarten auf diesem Computer fehlen. Klicken Sie auf **PDF trotzdem erstellen**, **Entwurfsprüfung öffnen** oder **Abbrechen**.

## Farben für eine Druckerei einrichten

Klicken Sie auf **Datei > Eigenschaften** und dann auf die Karte **Einstellungen für professionellen Druck**:

- Als **Farbmodell** stehen **RGB**, **Einzelfarbe (Volltonfarbe)**, **Volltonfarben**, **Prozessfarben (CMYK)** und **Prozess- und Volltonfarben** zur Verfügung. Prozessfarben sind die Druckfarben Cyan, Magenta, Gelb und Schwarz. Eine Volltonfarbe ist eine Sonderfarbe, die die Druckerei anmischt. Fügen Sie jede Volltonfarbe unter **Volltonfarben** hinzu.
- **Schwarz in Dateien für eine Druckerei überdrucken (PDF/X)** sorgt dafür, dass Schwarz über die darunterliegenden Farben gedruckt wird, sodass eine verschobene Druckplatte keinen weißen Rand hinterlässt. Sie legen fest, wofür dies gilt: Text unterhalb einer bestimmten Größe, Linien oder Füllungen.

## Verwandte Themen

- [Publikation drucken](printing)
- [Als Bild, Webseite oder E-Book speichern](export)
- [Entwurf prüfen](design-checker)
