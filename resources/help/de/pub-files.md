# Mit .pub-Dateien arbeiten
<!-- keywords: pub, .pub, pub-Datei, pub öffnen, pub speichern, konvertieren, Kompatibilität, importieren, alte Publikationen, andere Programme, jpub, Format, fehlende Schriftarten, Original sichern, Dateiformat, Dateien übernehmen -->

Eine .pub-Datei ist eine Publikation, die mit anderen Programmen für Desktop-Publishing erstellt wurde. JeffPub öffnet .pub-Dateien, die seit 1998 erstellt wurden. Ihre Arbeit können Sie auch als .pub-Datei speichern. Das eigene Format von JeffPub ist .jpub.

## Eine .pub-Datei öffnen

1. Drücken Sie Strg+O, oder klicken Sie auf **Datei > Öffnen**.
2. Klicken Sie auf **Durchsuchen**, wählen Sie die .pub-Datei aus, und klicken Sie auf **Öffnen**.

Sie können die Datei auch auf die Seite ziehen. Text, Bilder, Formen und Tabellen werden übernommen, ebenso verknüpfte Textfelder. Die meisten Seiten sehen dem Original nahe, aber nicht jedes Detail stimmt schon überein. Bei einigen Textfeldern endet eine Zeile vielleicht ein oder zwei Wörter früher oder später als vorher.

Wenn eine Schriftart der Datei auf Ihrem Computer fehlt, verwendet JeffPub stattdessen eine ähnliche installierte Schriftart. In der .pub-Datei eingebettete Schriftarten werden nicht übernommen. Wenn JeffPub nach dem Öffnen etwas mitteilen möchte, erscheint ein Hinweis in der Statusleiste am unteren Fensterrand.

Lässt sich die Datei nicht öffnen, teilt JeffPub mit, dass sie möglicherweise beschädigt ist oder Funktionen verwendet, die JeffPub noch nicht lesen kann.

## Ergebnis prüfen

Das Öffnen ist eine Konvertierung. Sehen Sie sich deshalb jede Seite an. Prüfen Sie Zeilenumbrüche, Schriftarten und Bilder. Mit der [Entwurfsprüfung](design-checker) finden Sie Text, der nicht passt.

## Als .pub-Datei speichern

- Klicken Sie auf **Datei > Exportieren** und dann auf **Als .pub-Datei speichern**.
- Oder klicken Sie auf **Datei > Speichern unter** und wählen bei **Dateityp** die Option **.pub-Publikationsdateien (*.pub)**.

Beim Speichern als .pub-Datei kann JeffPub Seiten, Textfelder und verknüpfte Textfelder, Textformatierungen, Formen, Linien, Bilder, Tabellen, Füllungen, Schatten, Textkunst, Formatvorlagen, Broschüren, Faltkarten, Umschläge, Seitenzahlen und Links schreiben. Hat ein Objekt keine .pub-Entsprechung, teilt JeffPub mit, wie viele Objekte es weggelassen hat. Diese Objekte bleiben in Ihrer Publikation erhalten und werden beim Speichern als .jpub-Datei mitgespeichert. Kommentare haben ebenfalls keine .pub-Entsprechung: JeffPub teilt mit, dass sie weggelassen wurden, und sie bleiben nur in .jpub-Dateien erhalten. Mehr dazu finden Sie im Hilfethema Kommentare.

## Original schützen

Wenn Sie eine .pub-Datei öffnen und Strg+S drücken, schreibt JeffPub die Änderungen in dieselbe .pub-Datei zurück. Um das Original zu sichern:

- Aktivieren Sie unter **Datei > Optionen** auf der Registerkarte **Speichern** die Option **Immer Sicherungskopie erstellen**. JeffPub behält dann die ältere Datei unter dem Namen „Sicherungskopie von“ und dem ursprünglichen Dateinamen.
- Oder speichern Sie zuerst über **Datei > Speichern unter** als .jpub-Datei. Eine .jpub-Datei behält alles bei.

Bewahren Sie Ihre Originaldateien auf, bis Sie sicher sind, dass die neuen richtig sind.

## Siehe auch

- [Publikationen öffnen und speichern](open-save)
- [Als Bild, Webseite oder E-Book speichern](export)
- [Einstellungen von JeffPub ändern](options)
