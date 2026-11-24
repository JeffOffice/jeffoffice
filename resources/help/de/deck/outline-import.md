# Folien aus einer Gliederung erstellen
<!-- keywords: Folien aus Gliederung, Gliederung importieren, Markdown, Textgliederung, RTF, DOCX, Überschriften zu Folien, Aufzählungsliste zu Folien, Präsentation aus Text beginnen, Gliederung speichern, Gliederung exportieren -->

Sie können eine Präsentation als Text schreiben und JeffDeck daraus Folien erstellen lassen. Jede Folie hat einen Titel und einen Textbereich im Layout „Titel und Inhalt“. Die Gliederungsansicht, „Folie zurücksetzen“ und Designs funktionieren daher wie gewohnt.

## Folien zur geöffneten Präsentation hinzufügen

1. Klicken Sie auf **Start > Folien**, dann auf den kleinen Pfeil unter **Neue Folie**, und wählen Sie **Folien aus Gliederung…**.
2. Wählen Sie eine Datei aus. Sie kann eine Textdatei (.txt), eine Markdown-Datei (.md), eine Rich-Text-Datei (.rtf) oder eine Dokumentdatei (.docx) sein.
3. Die Folien werden nach der aktuellen Folie eingefügt. Sie bilden einen einzigen Schritt, daher nehmen Sie mit einem Klick auf **Strg+Z** alle wieder zurück.

## Oder eine neue Präsentation daraus beginnen

Wählen Sie **Datei > Öffnen** und die Gliederungsdatei aus. JeffDeck erstellt daraus eine neue Präsentation. Die Datei wird nie verändert, und beim Klick auf **Speichern** werden Sie nach einem Namen gefragt.

## So wird die Datei gelesen

**Textdateien.** Eine Zeile, die am linken Rand beginnt, ist der Titel einer Folie. Die Zeilen darunter sind der Text der Folie. Eine Zeile, die weiter eingerückt ist als die vorherige, steht eine Ebene tiefer (verwenden Sie Tabstopps oder Leerzeichen). Ein Aufzählungszeichen (-, * oder +) oder eine Zahl am Zeilenanfang wird entfernt, weil die Folie eigene Aufzählungszeichen hat.

**Markdown-Dateien.** Die oberste Überschriftenebene der Datei bestimmt die Folien: # , wenn die Datei sie verwendet, andernfalls ##. Kleinere Überschriften werden zu Textzeilen der Folie, für jede Stufe eine Ebene tiefer. Eine Zeile mit drei Bindestrichen (---) beginnt eine neue Folie. Listen werden zu Ebenen. **Fett**, *kursiv*, `Code` und Links behalten ihr Aussehen. Ein Codeblock zwischen Zeilen mit drei Backticks wird zu einem Textfeld in einer Nichtproportionalschrift. Ein Bild in einer eigenen Zeile, in der Bildform von Markdown geschrieben (ein Ausrufezeichen, die Beschreibung in eckigen Klammern und der Dateiname in runden Klammern), wird auf der Folie platziert, aber nur, wenn die Bilddatei im selben Ordner wie die Gliederung liegt oder in einem Ordner darin. Bilder aus dem Internet oder aus anderen Ordnern werden nie geladen.

**Rich-Text- und .docx-Dateien.** Absätze in einer Überschriftenformatvorlage erstellen die Folien: Die oberste Überschriftenformatvorlage der Datei ist der Titel, kleinere Überschriften sind Textzeilen, und jeder andere Absatz ist eine Zeile unter der Überschrift darüber.

## Grenzwerte

Diese Grenzwerte gelten für alle Arten von Gliederungsdateien, Text, Markdown, Rich Text und .docx gleichermaßen:

- Bis zu 5.000 Folien. Alles darüber hinaus wird weggelassen.
- Bis zu 4.000 Zeichen pro Zeile. Eine längere Zeile wird abgeschnitten, ebenso ein Markdown-Absatz, dessen Zeilen zusammen länger werden.
- Bis zu 500 Textzeilen pro Folie und insgesamt 100.000 Zeilen.
- Der Text einer Folie hat bis zu 9 Ebenen. Eine Zeile, die tiefer eingerückt ist, wird auf der tiefsten Ebene angezeigt.
- Bis zu 200 Codezeilen pro Folie.
- Insgesamt bis zu 100 Bilder und höchstens 4 pro Folie. Eine Bilddatei darf bis zu 32 MB groß sein, alle Bilddateien zusammen bis zu 64 MB. Ein Bild muss eine PNG-, JPEG-, GIF-, BMP-, WebP- oder TIFF-Datei sein, unabhängig vom Dateinamen, mit höchstens 16 Millionen Pixeln (4.000 × 4.000 Pixel). Alle Bilder zusammen dürfen höchstens 64 Millionen Pixel haben.

Auch die Dateigröße selbst ist begrenzt. Eine Text- oder Markdown-Datei darf bis zu 10 MB groß sein und in UTF-8 oder UTF-16 vorliegen. Eine Rich-Text- oder .docx-Datei darf bis zu 96 MB groß sein. Eine Datei, die kein Text ist, wird abgelehnt. Bilder in einer Rich-Text- oder .docx-Datei werden nicht auf die Folien übernommen, und JeffDeck teilt Ihnen mit, wie viele weggelassen wurden.

Wenn etwas weggelassen wird, etwa ein Bild, das nicht verwendet werden konnte, oder Text über einer Grenze, zeigt JeffDeck an, was es war. Während eine große Datei gelesen wird, arbeitet JeffDeck weiter. Wenn Sie das Fenster schließen, wird das Einlesen abgebrochen.

## Als Gliederung speichern

Wählen Sie **Datei > Kopie speichern** und dann **Markdown-Gliederung (*.md)**. Die Datei enthält den Titel und den Text jeder Folie, mit Fett, Kursiv und Links. Bilder, Formen, Tabellen, Diagramme, andere Textfelder, Sprechernotizen und die Gestaltung der Folien werden nicht übernommen. JeffDeck weist beim Speichern darauf hin, und **Datei > Info > Kompatibilität prüfen** führt es ebenfalls auf.

## Siehe auch

- [Folien hinzufügen, ändern und löschen](slides)
- [Folien aus einer anderen Präsentation wiederverwenden](reuse)
- [Ansichten](views)
