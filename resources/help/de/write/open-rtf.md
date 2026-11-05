# RTF-Dateien öffnen
<!-- keywords: rtf, Rich Text, Rich Text Format, rtf öffnen, .rtf, ältere Dokumente, importieren, öffnen, konvertieren -->

**RTF** (Rich Text Format) ist ein Dokumentdateiformat, das fast jede Textverarbeitung lesen und schreiben kann. JeffWrite öffnet **.rtf**-Dateien und behält dabei Text und Erscheinungsbild bei.

## Eine RTF-Datei öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen** und dann auf **Durchsuchen**. Wählen Sie in der Liste der Dateitypen **Rich Text Format (*.rtf)**, um nur diese Dateien anzuzeigen, oder wählen Sie die Datei in der Liste aller Dokumente aus. Sie können eine .rtf-Datei auch auf ein JeffWrite-Fenster ziehen oder deren Namen beim Start von JeffWrite über die Befehlszeile angeben.

JeffWrite erkennt eine RTF-Datei an ihrem Inhalt und nicht an ihrem Namen. Eine Datei mit einer anderen Endung wird daher trotzdem geöffnet, wenn sie tatsächlich eine RTF-Datei ist.

## Was übernommen wird

JeffWrite liest diese Teile einer RTF-Datei:

- Text in beliebiger Sprache mit Schriftarten, Schriftgrößen, Farben, Fett, Kursiv, Unterstreichung, Durchstreichung, Hochstellung, Tiefstellung und Markierung.
- Absatzeinstellungen: Ausrichtung, Einzüge, Abstände, Tabstopps, Rahmen, Schattierung sowie die Einstellungen „Mit nächstem Absatz“ und „Zeilen zusammenhalten“.
- Formatvorlagen, nummerierte Listen und Aufzählungslisten.
- Tabellen, auch Tabellen in Tabellen, mit Rahmen, Schattierung, Spaltenbreiten und verbundenen Zellen.
- Seitengröße, Seitenränder, Spalten, Seitenausrichtung, Abschnitte, Kopf- und Fußzeilen.
- Fußnoten, Endnoten, Seitenzahlfelder, andere Felder und Textmarken.
- Nachverfolgte Änderungen: eingefügter und gelöschter Text, eingefügte oder gelöschte Absatzmarken sowie geänderte Formatierungen. Eine **nachverfolgte Änderung** ist eine Bearbeitung, die ein Prüfer markiert hat. Siehe [Änderungen nachverfolgen, annehmen und ablehnen](track-changes).
- Bilder im Format PNG, JPEG, EMF und WMF sowie Seitenwechsel und Spaltenwechsel.

Wenn ein Teil der Datei nicht gelesen werden kann, zeigt JeffWrite an, was ausgelassen wurde. Manche RTF-Dateien enthalten Elemente, die ein Dokument in JeffWrite noch nicht speichern kann, etwa Zeichnungen oder Formeln. Diese Elemente werden ausgelassen, der umgebende Text bleibt erhalten.

## Änderungen speichern

Sie können ein Dokument als .rtf-Datei speichern: Wählen Sie unter **Speichern unter** den Eintrag **Rich Text Format (*.rtf)**, oder drücken Sie **Strg+S** bei einem Dokument, das aus einer .rtf-Datei stammt. Die Datei behält Text und Erscheinungsbild, Formatvorlagen, Listen, Tabellen, Seiteneinrichtung, Kopf- und Fußzeilen, Verknüpfungen und Bilder (PNG, JPEG und EMF). Fußnoten, Endnoten, Kommentare, nachverfolgte Änderungen, Formeln, Formen und Textfelder bleiben nicht erhalten. JeffWrite zeigt beim Speichern an, was ausgelassen wird.

## Siehe auch

- [Öffnen, Speichern, Drucken und Exportieren](open-save)
- [Willkommen bei JeffWrite](start)
