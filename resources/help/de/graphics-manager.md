# Grafik-Manager
<!-- keywords: Grafik-Manager, Bilder, Grafiken, verknüpftes Bild, mit Datei verknüpfen, Verknüpfung aktualisieren, Verknüpfung ändern, Bild einbetten, eingebettet, fehlt, fehlendes Bild, geändert, nicht aktualisiert, außerhalb des Ordners, neu verknüpfen, Bilddatei suchen, Bildstatus, Auflösung -->

Der **Grafik-Manager** listet alle Bilder der Publikation auf und zeigt Ihnen, woher jedes Bild stammt. Am wichtigsten ist er für Bilder, die Sie mit ihren Dateien verknüpft haben, denn ein verknüpftes Bild hängt von einer Datei außerhalb der Publikation ab. Wie Sie ein Bild verknüpfen, erfahren Sie unter [Bilder hinzufügen und anpassen](pictures).

## Grafik-Manager öffnen

Klicken Sie auf **Ansicht > Anzeigen > Grafik-Manager**. Der Aufgabenbereich wird rechts geöffnet. Klicken Sie erneut auf dieselbe Schaltfläche oder oben im Aufgabenbereich auf das **X**, um ihn zu schließen.

## Die Liste lesen

Jede Zeile zeigt eine kleine Vorschau, den Dateinamen, die Seite, den Dateityp, die Größe in KB und den **Status**. Mit dem Feld über der Liste sortieren Sie die Zeilen nach Seite, Name, Größe oder Typ. Unter der Liste stehen die Details zum ausgewählten Bild: sein Status, seine Datei, ob die Publikation das vollständige Bild enthält, seine Größe in Pixeln und seine **effektive Auflösung**, also wie viele Pixel des Bilds auf jeden Zoll der Seite kommen. Etwa 300 ist gut für den Druck.

| Status | Bedeutung |
|---|---|
| Eingebettet | Das Bild ist in der Publikation gespeichert. Außerhalb der Publikation wird nichts benötigt. |
| Verknüpft | Das Bild ist mit einer Datei verknüpft, und die Datei befindet sich unverändert dort, wo JeffPub sie erwartet. |
| Fehlt | Das Bild ist mit einer Datei verknüpft, die nicht mehr vorhanden ist. Enthält die Publikation keine Kopie, zeigt die Seite die kleine Vorschau, die unscharf aussieht, und diese wird auch gedruckt. |
| Geändert | Die Datei wurde geändert, seit die Verknüpfung erstellt oder zuletzt aktualisiert wurde. Klicken Sie auf **Verknüpfung aktualisieren**, um die Änderungen zu übernehmen. |
| Nicht aktualisiert | Das Bild ist mit einer Datei außerhalb des Ordners der Publikation verknüpft, deshalb hat JeffPub diese Datei nicht geöffnet. Die Seite zeigt, was in der Publikation gespeichert ist. Klicken Sie auf **Verknüpfung aktualisieren**, um die Datei zu verwenden. Siehe „Bilder von außerhalb des Publikationsordners“ weiter unten. |

## Die Schaltflächen

- **Gehe zu** markiert das Bild auf seiner Seite.
- **Speichern unter…** speichert eine Kopie des Bilds in einer Datei.
- **Ersetzen…** setzt eine andere Datei in denselben Rahmen.
- **Verknüpfung aktualisieren** liest die Datei erneut ein. Verwenden Sie diese Schaltfläche bei einem Bild mit dem Status **Geändert**, nachdem Sie eine **fehlende** Datei zurückgelegt haben, oder bei einem Bild mit dem Status **Nicht aktualisiert**. Alle Bilder mit derselben Verknüpfung werden aktualisiert, und das Bild behält Position und Breite.
- **Verknüpfung ändern…** verknüpft das Bild mit einer anderen Datei, zum Beispiel wenn Sie die Dateien in einen anderen Ordner verschoben haben.
- **Bild einbetten** wandelt die Verknüpfung in eine Kopie um, die in der Publikation gespeichert wird. Das Bild hat dann den Status **Eingebettet**. Fehlt die Datei, kann nur die kleine Vorschau eingebettet werden; die Statusleiste weist Sie darauf hin.

Die drei Verknüpfungsschaltflächen sind abgeblendet, wenn das ausgewählte Bild nicht verknüpft ist. Jede Aktion ist ein einzelner Schritt, daher können Sie sie mit **Strg+Z** rückgängig machen.

## Bilder von außerhalb des Publikationsordners

Beim Öffnen einer Publikation liest JeffPub die Dateien der verknüpften Bilder, die sich im selben Ordner wie die Publikation oder in einem Unterordner davon befinden. Dateien an anderen Orten, auf Ihrem Computer oder im Netzwerk, werden nicht berücksichtigt.

Der Grund: Eine Publikation, die Ihnen jemand sendet, kann beliebige Dateien angeben. Würde JeffPub alles öffnen, was die Publikation angibt, könnte die Publikation eines Fremden eines Ihrer privaten Bilder auf ihre Seiten holen, das Sie dann womöglich unbemerkt speichern oder zurücksenden. Im Netzwerk kann zudem schon die Prüfung, ob eine Datei vorhanden ist, Ihre Windows-Anmeldedaten an einen anderen Computer senden.

Ein Bild, das mit einer Datei an einem anderen Ort verknüpft ist, zeigt daher, was in der Publikation gespeichert ist: das vollständige Bild, sofern die Publikation eines enthält, andernfalls die kleine Vorschau, die unscharf aussieht. Sein Status ist **Nicht aktualisiert**, und die Details geben an, dass sich die Datei außerhalb des Ordners befindet.

Um die Datei zu verwenden, wählen Sie das Bild aus und klicken auf **Verknüpfung aktualisieren**, oder Sie klicken auf **Verknüpfung ändern…** und wählen die Datei selbst aus. Damit erteilen Sie die Erlaubnis für genau dieses Bild, und sie gilt, bis Sie die Publikation schließen. Andere Bilder bleiben **Nicht aktualisiert**, bis Sie für jedes dasselbe tun.

Beim Speichern bleibt die Verknüpfung unverändert, sodass das Bild beim nächsten Öffnen der Publikation wieder **Nicht aktualisiert** ist. Wenn Bilder automatisch geladen werden sollen, legen Sie sie im Ordner der Publikation oder in einem Unterordner davon ab.

Beim Drucken, beim Speichern als PDF und beim Speichern als E-Book wird verwendet, was die Seite zeigt. Ist ein Bild **Nicht aktualisiert**, klicken Sie vor dem Drucken auf **Verknüpfung aktualisieren**. Die [Entwurfsprüfung](design-checker) listet diese Bilder auf.

## Ein fehlendes Bild reparieren

1. Wählen Sie in der Liste das Bild mit dem Status **Fehlt** aus.
2. Klicken Sie auf **Verknüpfung ändern…**, und wählen Sie die Datei aus. Oder verschieben Sie die Datei an ihren ursprünglichen Ort zurück, und klicken Sie auf **Verknüpfung aktualisieren**.
3. Ist die Datei endgültig verloren, klicken Sie auf **Bild einbetten**, um die Vorschau zu behalten, oder auf **Ersetzen…**, um ein anderes Bild zu verwenden.

Die [Entwurfsprüfung](design-checker) listet ebenfalls fehlende verknüpfte Bilder auf.

## Siehe auch

- [Bilder hinzufügen und anpassen](pictures)
- [Entwurf prüfen](design-checker)
- [Als Bild, Webseite oder E-Book speichern](export)
