# Lineale, Führungslinien und Ausrichten
<!-- keywords: Lineal, Nullpunkt, Linealnullpunkt, Ursprung, Führungslinie, Hilfslinie, Linealführungslinie, Randführungslinie, Raster, Spalten, Grundlinie, Grundlinienraster, Einrasten, magnetisch, an Führungslinien ausrichten, an Objekten ausrichten, Maßeinheiten, Zoll, Zentimeter, Führungslinien anzeigen, Begrenzungen -->

Führungslinien sind Linien, mit denen Sie Elemente aneinander ausrichten. Sie werden auf dem Bildschirm angezeigt, aber nie gedruckt. Durch das Einrasten springen Objekte an eine Führungslinie oder ein anderes Objekt, wenn sie in deren Nähe kommen.

## Lineale und Maßeinheiten

Mit **Ansicht > Anzeigen > Lineale** blenden Sie die Lineale ein oder aus. Eine rote Linie auf jedem Lineal folgt dem Mauszeiger, und der Bereich des Lineals über dem markierten Objekt ist schattiert. Während Sie in einem Textfeld schreiben, zeigt das obere Lineal Einzugs- und Tabstoppmarken.

Um die Maßeinheit zu ändern, klicken Sie auf **Datei > Optionen**, öffnen die Registerkarte **Erweitert** und wählen unter **Maßeinheiten** Zoll, Zentimeter, Millimeter, Punkt oder Pica. In jedes Feld für Größe oder Position können Sie auch eine Einheit eingeben, etwa 2 cm.

## Nullpunkt verschieben

Jedes Lineal zählt von der oberen linken Ecke der Seite aus: 0 ist die Ecke, und die Zahlen werden größer, je weiter Sie sich davon entfernen. Wenn Sie von einer anderen Stelle aus messen möchten, etwa von der Ecke eines Objekts, verschieben Sie den Nullpunkt:

- Halten Sie die **Umschalttaste** gedrückt, und ziehen Sie mit der **rechten** Maustaste entlang eines Lineals. Der Nullpunkt dieses Lineals wird an die Stelle verschoben, an der Sie loslassen.
- Um beide Lineale gleichzeitig zu verschieben, ziehen Sie aus dem kleinen Feld, in dem sich die beiden Lineale treffen.
- Um den Nullpunkt eines Lineals wieder auf die Seitenecke zu setzen, doppelklicken Sie auf dieses Lineal. Für beide Lineale doppelklicken Sie auf das kleine Feld. (Während Sie in einem Textfeld schreiben, öffnet ein Doppelklick auf das obere Lineal stattdessen die Einstellungen für **Tabstopps**.)

Die Zahlen und Teilstriche auf den beiden Seitenlinealen zählen vom Nullpunkt aus, ebenso die Positionsangabe in der Statusleiste. Liegt der Nullpunkt 7,5 cm vom linken Rand entfernt, wird für ein Objekt 2,5 cm vom linken Seitenrand dort -5 cm angezeigt, weil es 5 cm vor dem Nullpunkt liegt. Die Positionsfelder (etwa im Fenster **Maßeinheit**), das Fenster **Linealführungslinien** und die Führungslinien, die Sie aus dem Lineal ziehen, messen weiterhin von der Seitenecke aus. Die Einzugs- und Tabstoppmarken auf dem Textlineal messen weiterhin vom Textfeld aus.

Der Nullpunkt wird mit der Publikation gespeichert und ist beim erneuten Öffnen der Datei wieder vorhanden. Eine neue Publikation beginnt mit beiden Nullpunkten an der Seitenecke. Das Verschieben des Nullpunkts lässt sich in einem Schritt **Rückgängig** machen.

## Die vier Arten von Führungslinien

- **Randführungslinien** (rosa) markieren die Seitenränder. Sie legen sie unter **Seitenentwurf > Seite einrichten > Seitenränder** fest.
- **Rasterführungslinien** (blau) teilen die Seite in Spalten und Zeilen. Sie gehören zu einer Masterseite.
- **Grundlinienführungslinien** (hellbraune gepunktete Linien) markieren die Linien, auf denen der Text steht.
- **Linealführungslinien** (grün) setzen Sie selbst.

Mit **Ansicht > Anzeigen > Führungslinien** blenden Sie alle ein oder aus. **Ansicht > Anzeigen > Grundlinien** (Strg+F7) blendet die Grundlinienführungslinien ein oder aus.

## Spalten und Zeilen festlegen

Klicken Sie auf **Seitenentwurf > Seite einrichten > Führungslinien**. Wählen Sie unter **Integrierte Führungslinien** die Option „Kein Raster“, „2 Spalten“, „3 Spalten“, „4 Spalten“ oder ein 2 × 2-, 3 × 3- oder 4 × 4-Raster. Für weitere Einstellungen wählen Sie **Raster- und Grundlinienführungslinien…**. Das Fenster **Layoutführungslinien** hat dann diese Registerkarten:

- **Randführungslinien**: die vier Seitenränder.
- **Rasterführungslinien**: Spalten, Zeilen und der Abstand dazwischen. Sie können auch eine mittlere Führungslinie hinzufügen.
- **Grundlinienführungslinien**: der Abstand zwischen den Linien und die Position der ersten Linie.

Damit Text auf den Grundlinienführungslinien steht, öffnen Sie das Fenster **Absatz** und aktivieren **Text an Grundlinienführungslinien ausrichten**.

## Linealführungslinien hinzufügen

- Ziehen Sie aus dem oberen Lineal, um eine horizontale Führungslinie zu erstellen, und aus dem linken Lineal für eine vertikale.
- Oder wählen Sie im Menü **Führungslinien** den Befehl **Horizontale Linealführungslinie hinzufügen** oder **Vertikale Linealführungslinie hinzufügen**. Jede neue Führungslinie wird neben der vorherigen eingefügt.
- Ziehen Sie eine Führungslinie, um sie zu verschieben. Ziehen Sie sie zurück auf ihr Lineal, um sie zu löschen.
- Mit **Linealführungslinien…** geben Sie genaue Positionen ein. Dort können Sie auch eine Reihe von Führungslinien in gleichmäßigem Abstand hinzufügen.
- **Alle Linealführungslinien löschen** entfernt die Führungslinien von der aktuellen Seite.

Führungslinien auf einer Masterseite werden auf jeder Seite angezeigt, die diese Masterseite verwendet. Um sie zu verschieben, wechseln Sie zur Masterseite.

## Einrasten ein- oder ausschalten

Verwenden Sie **Seitenentwurf > Layout**:

- **An Führungslinien ausrichten**: Objekte rasten an Rand-, Raster- und Linealführungslinien, an den Seitenkanten und an der Seitenmitte ein.
- **An Objekten ausrichten**: Objekte rasten an den Kanten und Mittelpunkten anderer Objekte ein.

Beide Optionen sind anfangs aktiviert. Das Einrasten greift, wenn Sie nur wenige Pixel entfernt sind. Halten Sie beim Ziehen die **Alt**-Taste gedrückt, um es für diese Bewegung auszuschalten.

## Verwandte Themen

- [Objekte verschieben, ausrichten, gruppieren und anordnen](arrange)
- [Masterseiten, Kopfzeilen und Seitenzahlen](master-pages)
- [Seitengröße, Seitenränder und Layout](page-setup)
- [Die Registerkarte „Seitenentwurf“](tab-page-design)
