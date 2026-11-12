# Bildschirmleser, hoher Kontrast und Alternativtext
<!-- keywords: Barrierefreiheit, Bildschirmleser, Erzähler, blind, Sehbehinderung, hoher Kontrast, Dunkelmodus, dunkles Design, Fokusrahmen, Alternativtext, Bildbeschreibung, Zoom, vergrößern, nur Tastatur, Screenreader -->

JeffPub ist so aufgebaut, dass auch Menschen, die Bildschirmleser, die Tastatur oder kontrastreiche Farben verwenden, damit arbeiten können.

## Bildschirmleser

Ein **Bildschirmleser** ist ein Programm, das den Bildschirm vorliest. JeffPub arbeitet mit Bildschirmlesern wie dem Erzähler unter Windows zusammen. Jedes Steuerelement hat einen Namen und eine Beschreibung. Dazu gehören die Registerkarten, Schaltflächen, Galerien und Farbschaltflächen des Menübands, die Liste der Seiten, die Seite, die Aufgabenbereiche und die Statusleiste.

Auch die Objekte auf der Seite werden vorgelesen. Drücken Sie auf der Seite **Tab**, um das nächste Objekt auszuwählen: Der Bildschirmleser sagt, um welche Art es sich handelt und was es enthält, etwa „Textfeld: Frühlingsangebot“ oder „Bild: ein rotes Bauernhaus“, danach, wo es sich befindet und wie groß es ist. Ein Bild ohne Alternativtext wird als „Bild, kein Alternativtext“ vorgelesen, sodass Sie wissen, welche Bilder noch einen brauchen. Drücken Sie **Eingabe**, um in einem Textfeld zu schreiben; der Bildschirmleser kann dann dessen Text, die Zeile oder das Wort mit der Einfügemarke und Ihre Auswahl vorlesen.

- Die Liste der Seiten liest jede Seite als „Seite 1“, „Seite 2“ und so weiter. Wenn Sie einer Seite einen Namen gegeben haben, folgt dieser, etwa „Seite 3, Umschlag“.
- Der Seitenbereich heißt „Seite“. Seine Beschreibung nennt die Tasten: Tab wählt das nächste Objekt aus, die Pfeiltasten verschieben es, Eingabe bearbeitet seinen Text, und Entf löscht es.
- Registerkarten werden als Registerkarten des Menübands vorgelesen. Registerkarten, die erscheinen, wenn Sie etwas auswählen, nennen zusätzlich ihre Gruppe, etwa Bildtools.

## Tastatur verwenden

Sie können das Menüband, die Liste der Seiten und die Objekte auf der Seite allein mit der Tastatur erreichen. Drücken Sie **F6**, um zwischen Menüband, Seitenliste und Seite zu wechseln. Drücken Sie **Alt**, um die Zugriffstasteninfos anzuzeigen. Ein farbiger Rahmen zeigt, welches Steuerelement den Fokus hat. Siehe [JeffPub mit der Tastatur verwenden](keyboard).

## Farben ändern

- **Hell oder dunkel**: Klicken Sie auf **Datei > Optionen** und stellen Sie auf der Registerkarte **Allgemein** unter **Design** die Option **Hell**, **Dunkel** oder **Systemeinstellung verwenden** ein. Die Änderung wirkt sofort.
- **Hoher Kontrast**: Wenn Sie unter Windows ein Design mit hohem Kontrast einschalten, übernimmt JeffPub es. Es verwendet die Farben, die Windows gewählt hat, und ändert sich, wenn Sie das Design ein- oder ausschalten.

## Bilder leichter verständlich machen

**Alternativtext** ist eine kurze Beschreibung eines Bildes oder eines anderen Objekts. Bildschirmleser lesen sie vor. JeffPub übernimmt sie auch in Webseiten und E-Books, die Sie speichern.

1. Klicken Sie mit der rechten Maustaste auf das Bild oder Objekt.
2. Klicken Sie auf **Objekt formatieren…**.
3. Klicken Sie auf die Registerkarte **Alternativtext**.
4. Geben Sie im Feld **Alternativtext** eine Beschreibung ein.
5. Klicken Sie auf **OK**.

Beschreiben Sie in wenigen Worten, was das Bild zeigt, etwa „Zwei Kinder pflanzen einen Baum“. Symbole und Barcodes, die Sie einfügen, erhalten selbst einen Alternativtext.

## Seite prüfen

Klicken Sie auf **Überprüfen > Barrierefreiheit prüfen**, um Inhalte aufzulisten, die für manche Menschen schwer nutzbar sein könnten. Außer Bildern und Formen ohne Alternativtext findet die Prüfung drei weitere Punkte.

- **Text, der schwer lesbar ist.** Text braucht genügend *Kontrast* zu dem, was dahinter liegt: zur Füllung seines Feldes oder zur Seitenfarbe, wenn das Feld keine Füllung hat. Die Prüfung berechnet den Kontrast als Verhältnis. Ein Verhältnis von 4,5 zu 1 reicht für normalen Text, 3 zu 1 für großen Text (18 Punkt oder mehr, bzw. 14 Punkt oder mehr fett). Grau #777777 auf Weiß ergibt beispielsweise 4,48 zu 1, knapp zu wenig, und #767676 ergibt 4,54 zu 1. Wählen Sie eine dunklere Textfarbe oder einen helleren Hintergrund. Liegt Text über einem Bild, kann die Prüfung das nicht beurteilen und gibt deshalb einen Hinweis, den Kontrast selbst zu prüfen.
- **Objekte werden in einer anderen Reihenfolge gelesen.** Ein Bildschirmleser und eine getaggte PDF-Datei lesen die Objekte einer Seite vom Ende des Stapels zum Anfang. Weicht diese Reihenfolge davon ab, dass sie von oben nach unten und dann von links nach rechts verläuft, listet die Prüfung die betroffenen Objekte als Hinweise auf. Klicken Sie auf **Lesereihenfolge**, um die Objekte in der Reihenfolge zu sehen, in der sie gelesen werden. Wählen Sie ein Objekt aus und klicken Sie auf **Früher lesen** oder **Später lesen**, oder ziehen Sie es. Jeder Schritt lässt sich mit **Rückgängig** zurücknehmen, und er ändert außerdem, welches Objekt vorne liegt.
- **Eine Tabelle ohne Kopfzeile.** Ein Bildschirmleser verwendet die erste Zeile einer Tabelle, um ihre Spalten zu benennen. Eine Tabelle, deren erste Zeile nicht als Kopfzeile markiert ist, wird als Warnung aufgeführt. Klicken Sie auf das Problem und dann auf **Erste Zeile als Kopfzeile verwenden**, um die Kopfzeile einzuschalten. Der Schritt lässt sich mit **Rückgängig** zurücknehmen.

## Seite vergrößern

Vergrößern Sie die Ansicht, um kleinen Text zu lesen. Verwenden Sie die Zoomsteuerung in der Statusleiste, halten Sie **Strg** gedrückt und drehen Sie das Mausrad, oder wählen Sie im Feld **Zoom** auf der Registerkarte **Ansicht** eine Größe. Der Zoomregler reicht von etwa 10 % bis 1600 %.

## Siehe auch

- [JeffPub mit der Tastatur verwenden](keyboard)
- [Bilder einfügen und anpassen](pictures)
- [JeffPub-Einstellungen ändern](options)
- [Als Bild, Webseite oder E-Book speichern](export)
