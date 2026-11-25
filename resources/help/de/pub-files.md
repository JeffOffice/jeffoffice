# Mit .pub-Dateien arbeiten
<!-- keywords: pub, .pub, pub-Datei, pub öffnen, pub speichern, konvertieren, Kompatibilität, importieren, alte Publikationen, andere Programme, jpub, Format, fehlende Schriftarten, Original sichern, Dateiformat, Dateiformat wechseln, Sicherungskopie, Datei umwandeln -->

Eine .pub-Datei ist eine Publikation, die mit anderen Desktop-Publishing-Programmen erstellt wurde. JeffPub öffnet .pub-Dateien, die seit 1998 erstellt wurden. Ihre Arbeit können Sie auch als .pub-Datei speichern. Das eigene Format von JeffPub ist .jpub.

## Eine .pub-Datei öffnen

1. Drücken Sie Strg+O, oder klicken Sie auf **Datei > Öffnen**.
2. Klicken Sie auf **Durchsuchen**, wählen Sie die .pub-Datei aus, und klicken Sie auf **Öffnen**.

Sie können die Datei auch auf die Seite ziehen. Text, Bilder, Formen und Tabellen werden übernommen, ebenso verknüpfte Textfelder. Die meisten Seiten sehen dem Original nahe, aber nicht jedes Detail stimmt schon überein. Bei einigen Textfeldern endet eine Zeile vielleicht ein oder zwei Wörter früher oder später als vorher.

Ist eine Schriftart der Datei auf Ihrem Computer nicht vorhanden, verwendet JeffPub stattdessen eine ähnliche freie Schriftart. In der .pub-Datei gespeicherte Schriftarten werden nicht übernommen. Hat JeffPub nach dem Öffnen etwas mitzuteilen, erscheint unten im Fenster in der Statusleiste ein Hinweis.

Lässt sich die Datei nicht öffnen, meldet JeffPub, dass sie möglicherweise beschädigt ist oder Funktionen verwendet, die JeffPub noch nicht lesen kann. Dasselbe gilt für eine Datei, die Text oder Formen enthält, von denen JeffPub aber keines auf einer Seite platzieren kann: Sie wird nicht als leere Seiten geöffnet, damit Sie sie nicht versehentlich überschreiben, und die Datei bleibt unverändert.

## Das Ergebnis prüfen

Da das Öffnen eine Umwandlung ist, sehen Sie sich jede Seite an. Prüfen Sie Zeilenumbrüche, Schriftarten und Bilder. Mit der [Entwurfsprüfung](design-checker) finden Sie Text, der nicht passt.

## Als .pub-Datei speichern

- Klicken Sie auf **Datei > Exportieren** und dann auf **Als .pub-Datei speichern**.
- Oder klicken Sie auf **Datei > Speichern unter** und wählen unter **Dateityp** die Option **.pub-Publikationsdateien (*.pub)**.

Beim Speichern als .pub-Datei kann JeffPub Seiten, Textfelder und verknüpfte Textfelder, Textformatierung, Formen, Linien, Bilder, Tabellen, Füllungen, Schatten, Textkunst, Formatvorlagen, Broschüren, Faltkarten, Umschläge, Seitenzahlen und Links schreiben. Hat etwas keine .pub-Form, meldet JeffPub, wie viele Objekte es weggelassen hat. Diese Objekte bleiben in Ihrer Publikation erhalten und werden beim Speichern als .jpub-Datei mitgespeichert. Kommentare haben ebenfalls kein .pub-Format: JeffPub meldet, dass sie nicht übernommen wurden, und sie bleiben nur in .jpub-Dateien erhalten. Mehr dazu finden Sie im Hilfethema Kommentare.

## Das Original schützen

Wenn Sie eine .pub-Datei öffnen und Strg+S drücken, schreibt JeffPub die Änderungen in dieselbe .pub-Datei zurück. So bleibt das Original sicher:

- Aktivieren Sie unter **Datei > Optionen** auf der Registerkarte **Speichern** die Option **Immer Sicherungskopie erstellen**. JeffPub behält dann die ältere Datei als „Sicherungskopie von“ mit dem ursprünglichen Namen.
- Oder beginnen Sie mit **Datei > Speichern unter** und speichern Sie als .jpub-Datei. Eine .jpub-Datei behält alles.

Bewahren Sie Ihre Originaldateien auf, bis Sie sicher sind, dass die neuen richtig sind.

## Siehe auch

- [Publikationen öffnen und speichern](open-save)
- [Als Bild, Webseite oder E-Book speichern](export)
- [JeffPub-Einstellungen ändern](options)
