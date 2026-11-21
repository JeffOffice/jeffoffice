# Bildschirmleser, hoher Kontrast und Alternativtext
<!-- keywords: Barrierefreiheit, Bildschirmleser, Erzähler, blind, sehbehindert, hoher Kontrast, Dunkelmodus, dunkles Design, Fokusrahmen, Alternativtext, Bildbeschreibung, Zoom, vergrößern, nur Tastatur, Screenreader -->

JeffPub ist so aufgebaut, dass Menschen, die Bildschirmleser, die Tastatur oder kontrastreiche Farben nutzen, damit ebenfalls arbeiten können.

## Bildschirmleser

Ein **Bildschirmleser** ist ein Programm, das den Bildschirm laut vorliest. JeffPub arbeitet mit Bildschirmlesern wie dem Erzähler unter Windows zusammen. Jedes Steuerelement hat einen Namen und eine Beschreibung. Dazu gehören die Registerkarten, Schaltflächen, Galerien und Farbschaltflächen des Menübands, die Liste der Seiten, die Seite, die Aufgabenbereiche und die Statusleiste.

Auch die Objekte auf der Seite werden vorgelesen. Drücken Sie auf der Seite **Tab**, um das nächste Objekt auszuwählen: Der Bildschirmleser sagt, um welche Art es sich handelt und was es enthält, etwa „Textfeld: Frühlingsverkauf“ oder „Bild: ein roter Schuppen“, danach, wo es liegt und wie groß es ist. Ein Bild ohne Alternativtext wird als „Bild, kein Alternativtext“ vorgelesen, sodass Sie erkennen, bei welchen noch einer fehlt. Drücken Sie **Eingabe**, um in ein Textfeld zu schreiben. Der Bildschirmleser kann dann dessen Text, die Zeile oder das Wort, in der bzw. dem sich der Cursor befindet, sowie die Auswahl vorlesen.

- Die Liste der Seiten liest jede Seite als „Seite 1“, „Seite 2“ und so weiter vor. Wenn Sie einer Seite einen Namen gegeben haben, folgt dieser, etwa „Seite 3, Titelseite“.
- Der Seitenbereich heißt „Seite“. Seine Beschreibung nennt Ihnen die Tasten: Tab wählt das nächste Objekt aus, die Pfeiltasten verschieben es, Eingabe bearbeitet seinen Text, und Entf entfernt es.
- Registerkarten werden als Registerkarten des Menübands vorgelesen. Registerkarten, die erscheinen, wenn Sie etwas auswählen, nennen zusätzlich ihre Gruppe, etwa Bildtools.

## Tastatur verwenden

Sie können das Menüband, die Liste der Seiten und die Objekte auf der Seite allein mit der Tastatur erreichen. Drücken Sie **F6**, um zwischen Menüband, Seitenliste und Seite zu wechseln. Drücken Sie **Alt**, um die Zugriffstasteninfos anzuzeigen. Ein farbiger Rahmen zeigt, welches Steuerelement den Fokus hat. Siehe [Mit der Tastatur in JeffPub arbeiten](keyboard).

## Farben ändern

- **Hell oder dunkel**: Klicken Sie auf **Datei > Optionen**, und stellen Sie auf der Registerkarte **Allgemein** unter **Design** die Option **Hell**, **Dunkel** oder **Systemeinstellung verwenden** ein. Die Änderung wirkt sofort.
- **Hoher Kontrast**: Wenn Sie unter Windows ein Design mit hohem Kontrast einschalten, übernimmt JeffPub es. Es verwendet die von Windows gewählten Farben und passt sich an, wenn Sie das Design ein- oder ausschalten.

## Bilder leichter verständlich machen

**Alternativtext** ist eine kurze Beschreibung eines Bildes oder eines anderen Objekts. Bildschirmleser lesen sie vor. JeffPub übernimmt sie außerdem in Webseiten und E-Books, die Sie speichern.

1. Klicken Sie mit der rechten Maustaste auf das Bild oder Objekt.
2. Klicken Sie auf **Objekt formatieren…**.
3. Klicken Sie auf die Registerkarte **Alternativtext**.
4. Geben Sie im Feld **Alternativtext** eine Beschreibung ein.
5. Klicken Sie auf **OK**.

Beschreiben Sie in wenigen Worten, was das Bild zeigt, etwa „Zwei Kinder pflanzen einen Baum“. Eingefügte Symbole und Barcodes erhalten Alternativtext automatisch.

## Seite prüfen

Klicken Sie auf **Überprüfen > Barrierefreiheit prüfen**, um aufzulisten, was für manche Menschen schwer zu bedienen sein kann. Neben Bildern und Formen ohne Alternativtext findet die Prüfung vier weitere Dinge.

- **Schwer lesbarer Text.** Text braucht genügend Kontrast zu dem, was dahinter liegt: zur Füllung seines Feldes oder zur Seitenfarbe, wenn das Feld keine Füllung hat. Die Prüfung berechnet den Kontrast als Verhältnis. Ein Verhältnis von 4,5 zu 1 reicht für normalen Text, 3 zu 1 für großen Text (18 Punkt oder mehr bzw. 14 Punkt oder mehr fett). Grauer Text #777777 auf Weiß ergibt etwa 4,48 zu 1 und liegt damit knapp darunter, #767676 ergibt 4,54 zu 1. Wählen Sie eine dunklere Textfarbe oder einen helleren Hintergrund. Liegt Text über einem Bild, kann die Prüfung dies nicht beurteilen und gibt deshalb einen Hinweis, den Kontrast selbst zu prüfen.
- **Objekte werden in einer anderen Reihenfolge gelesen.** Ein Bildschirmleser und eine getaggte PDF-Datei lesen die Objekte einer Seite vom Ende des Stapels bis zur Vorderseite. Weicht diese Reihenfolge davon ab, dass oben vor unten und links vor rechts gelesen wird, listet die Prüfung die falsch platzierten Objekte als Hinweise auf. Klicken Sie auf **Lesereihenfolge**, um die Objekte in der Reihenfolge zu sehen, in der sie gelesen werden. Wählen Sie ein Objekt aus, und klicken Sie auf **Früher lesen** oder **Später lesen**, oder ziehen Sie es. Jede Verschiebung ist ein Schritt von **Rückgängig**, und sie ändert auch, welches Objekt vorne liegt.
- **Eine Tabelle ohne Kopfzeile.** Ein Bildschirmleser verwendet die erste Zeile einer Tabelle, um deren Spalten zu benennen. Eine Tabelle, deren erste Zeile nicht als Kopfzeile markiert ist, wird als Warnung aufgeführt. Klicken Sie auf das Problem und dann auf **Erste Zeile als Kopfzeile verwenden**, um die Kopfzeile einzuschalten. Dies ist ein Schritt von **Rückgängig**.
- **Eine Tabelle mit verbundenen Zellen.** Ein Bildschirmleser kann in einer Tabelle die Orientierung verlieren, wenn Zellen verbunden sind. Deshalb wird eine Tabelle mit einer verbundenen Zelle als Warnung aufgeführt. Lassen Sie jede Zelle einer Tabelle unverbunden: Heben Sie die Verbindung auf, oder verwenden Sie zwei Tabellen.
- **Ein Link, der nicht sagt, wohin er führt.** Ein Bildschirmleser kann die Links einer Seite als Liste vorlesen. Daher sollte der Text jedes Links die Seite benennen, die er öffnet. Ein Link, dessen Text seine eigene Adresse ist, nur „hier“, „klicken Sie hier“, „weiterlesen“ oder ein ähnliches Wort lautet oder leer ist, wird als Warnung aufgeführt. Ändern Sie den Text, etwa in „Unser Jahresbericht“ statt „klicken Sie hier“. Die ersten 50 werden einzeln aufgeführt, die übrigen werden in einer Zeile gezählt.

## Seite vergrößern

Vergrößern Sie die Ansicht, um kleinen Text zu lesen. Verwenden Sie die Steuerelemente **Zoom** in der Statusleiste, halten Sie **Strg** gedrückt und drehen Sie das Mausrad, oder wählen Sie eine Größe im Feld **Zoom** auf der Registerkarte **Ansicht**. Der Zoomregler reicht von etwa 10 % bis 1600 %.

## Siehe auch

- [Mit der Tastatur in JeffPub arbeiten](keyboard)
- [Bilder einfügen und anpassen](pictures)
- [JeffPub-Einstellungen ändern](options)
- [Als Bild, Webseite oder E-Book speichern](export)
