# Präsentationen öffnen und speichern
<!-- keywords: Makros, makrofähig, pptm, potm, ppsm, öffnen, speichern, speichern unter, otp, LibreOffice, weggelassen, schließen, neu, zuletzt verwendet, angeheftet, Dateitypen, pptx, potx, ppsx, ppt, odp, Vorlage, Präsentation, umbenennen, Strg+S, Sicherung, Sicherungskopie, Dateiformate, alte Dateien, OpenDocument-Präsentation, beibehalten, nicht angezeigt, Schriftarten, Schriftarten einbetten, eingebettete Schriftarten, fehlende Schriftart, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie erstellen, Meine Vorlagen, als Vorlage speichern, reparieren, Öffnen und reparieren, beschädigt -->

JeffDeck speichert Ihre Arbeit in .pptx-Dateien und in .odp-Dateien und öffnet mehrere Arten von Präsentationsdateien.

## Eine neue Präsentation beginnen

Drücken Sie Strg+N. Ein neues Fenster öffnet sich mit einer leeren Präsentation. Oder klicken Sie auf **Datei > Neu** und dann auf **Leere Präsentation**. Wenn das Fenster, in dem Sie gerade arbeiten, eine neue Präsentation enthält, die noch niemand bearbeitet hat, ersetzt die leere Präsentation sie. Andernfalls öffnet sie sich in einem eigenen Fenster.

## Speichern

Drücken Sie Strg+S, oder klicken Sie auf **Datei > Speichern**. Beim ersten Mal fragt JeffDeck nach einem Namen und einem Speicherort. Danach ersetzt **Speichern** die Datei durch Ihre neueste Fassung. Eine Meldung unten zeigt „Gespeichert“ und den Namen der Datei an.

Ein Stern (*) nach dem Namen in der Titelleiste bedeutet, dass Sie Änderungen haben, die nicht gespeichert sind. Speichern Sie häufig. Wenn JeffDeck unerwartet beendet wird, lesen Sie [Nicht gespeicherte Arbeit wiederherstellen](recover).

Wenn eine Meldung besagt, dass einige Objekte „in dieser Dateiart noch nicht gespeichert werden können“, wurde die Datei gespeichert, JeffDeck konnte diese Objekte aber nicht schreiben. Bewahren Sie auch die ursprüngliche Datei auf. Wenn Sie eine .odp-Datei speichern, listet die Meldung auf, was diese Dateiart nicht enthalten kann. Der Abschnitt „Als .odp-Datei speichern“ weiter unten erklärt, was sie nicht enthalten kann.

## Mit neuem Namen oder in anderer Dateiart speichern

Klicken Sie auf **Datei > Speichern unter** oder drücken Sie F12. Geben Sie einen Namen ein, wählen Sie die Dateiart und klicken Sie auf **Speichern**. Zur Auswahl stehen:

- **Präsentation (*.pptx)** ist die übliche Wahl.
- **Vorlage (*.potx)** speichert die Folien als Muster, aus dem neue Präsentationen begonnen werden.
- **Bildschirmpräsentation (*.ppsx)** speichert eine Präsentation, die abgespielt werden soll.
- **Präsentation mit Makros (*.pptm)**, **Vorlage mit Makros (*.potm)** und **Bildschirmpräsentation mit Makros (*.ppsm)** sind dieselben drei Arten für eine Präsentation mit **Makros**. Ein Makro ist ein kleines Programm, das zu einer Datei gehört. JeffDeck führt Makros nie aus und liest oder zeigt sie auch nicht an. Wenn Sie eine dieser Dateien öffnen und in derselben Art speichern, bleiben die Makros unverändert erhalten. Wenn Sie sie als .pptx, .potx oder .ppsx speichern, werden die Makros weggelassen, und JeffDeck weist Sie darauf hin. Eine Präsentation ohne Makros lässt sich ebenfalls in diesen Arten speichern, hat aber keine Makros, die erhalten bleiben könnten. Sind die Makros in einer Datei zu groß, um sie zu behalten (mehr als 32 MB in einem Teil oder 64 MB insgesamt), lässt JeffDeck sie weg und meldet dies beim Öffnen.
- **OpenDocument-Präsentation (*.odp)** speichert die Folien in der Dateiart, die LibreOffice und andere freie Programme verwenden.
- **OpenDocument-Präsentationsvorlage (*.otp)** ist die Vorlagenform dieser Art.

Wenn Sie die Endung weglassen, ergänzt JeffDeck die zur Dateiart passende Endung. Nach dem Speichern zeigt die Titelleiste den neuen Namen, und Sie arbeiten in der neuen Datei weiter.

Eine Präsentation, die Sie aus einer Vorlage geöffnet haben, wird als neue Präsentation gespeichert: **Speichern** fragt nach einem Namen, damit die Vorlage unverändert bleibt. Dasselbe gilt für eine Präsentation, die Sie aus einer älteren .ppt-, .pps- oder .pot-Datei geöffnet haben. Eine Präsentation, die Sie aus einer .odp-Datei geöffnet haben, wird wieder als .odp-Datei und über dieselbe Datei gespeichert.

## Eine Kopie speichern

Klicken Sie auf **Datei > Kopie speichern** und wählen Sie einen Namen. JeffDeck schreibt die Präsentation in diese Datei, und Sie arbeiten in der bisherigen weiter. Ihr Name und Ihre nicht gespeicherten Änderungen bleiben unverändert.

## Eine Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen** und aktivieren Sie auf der Registerkarte **Allgemein** die Option **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffDeck zuerst die alte Datei in eine Datei mit dem Präfix „Sicherungskopie von“ und dem Namen der Datei, im selben Ordner. Lässt sich die Kopie nicht anlegen, fragt das Programm, ob ohne Kopie gespeichert werden soll. Die Antwort lautet Nein, sofern Sie nicht Ja sagen, und die Datei bleibt dann unverändert.

## Als Vorlage speichern

Eine Vorlage ist eine Präsentation, die Sie als Ausgangspunkt verwenden. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage (eine **.potx**-Datei) landet unter **Meine Vorlagen**, und Ihre Präsentation bleibt unverändert. Um eine neue Präsentation aus der Vorlage zu beginnen, klicken Sie auf **Datei > Neu** und dann auf die Vorlage unter **Meine Vorlagen**. Die neue Präsentation hat noch keine eigene Datei, daher bleibt die Vorlage unverändert. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Öffnen

Drücken Sie Strg+O, um sofort eine Datei auszuwählen. Oder klicken Sie auf **Datei > Öffnen** und dann auf **Durchsuchen** oder auf einen Namen unter **Angeheftet** oder **Zuletzt verwendete Präsentationen**.

Die Datei öffnet sich in diesem Fenster, wenn es eine neue Präsentation enthält, die noch niemand bearbeitet hat. Andernfalls öffnet sie sich in einem eigenen Fenster.

Klicken Sie mit der rechten Maustaste auf einen Namen in der Liste, um ihn **An Liste anheften** (er bleibt oben), **Von Liste lösen** oder **Aus Liste entfernen**. Ein grau dargestellter Name bedeutet, dass JeffDeck die Datei aktuell nicht finden kann.

## Andere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Außerdem können Sie mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken.

- **Schreibgeschützt öffnen** öffnet die Präsentation nur zum Lesen. Sie können sich die Folien ansehen, durch sie blättern und Inhalte kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** drücken, fragt JeffDeck nach einem neuen Namen, sodass die geöffnete Datei unverändert bleibt.
- **Als Kopie öffnen** öffnet die Folien als neue Präsentation ohne eigene Datei. **Speichern** fragt, wohin sie gelegt werden soll.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .pptx-Datei, zum Beispiel eine, die beim Kopieren abgeschnitten wurde. JeffDeck liest jeden Teil, den es lesen kann, behält einen Teil, der mitten im Vorgang abbrach, bis zu der Stelle, an der er abbrach, und lässt einen Teil weg, den es nicht lesen kann. Die reparierten Folien öffnen sich als Kopie mit dem Namen „Repariert“ und dem ursprünglichen Namen. Eine Meldung nennt, was weggelassen wurde. Die beschädigte Datei bleibt unverändert.

## Dateien mit Kennwort

Eine **.pptx**- oder **.odp**-Datei, die ein Kennwort verlangt, öffnet sich wie gewohnt: JeffDeck fragt nach dem Kennwort, und Sie können es erneut versuchen, falls es nicht stimmt, oder auf **Abbrechen** klicken. Die Datei bleibt beim Speichern geschützt. Um das Kennwort zu ändern oder zu entfernen, verwenden Sie **Datei > Informationen > Mit Kennwort verschlüsseln** (siehe Der Bereich „Datei“). Eine als .odp-Datei gespeicherte Kopie behält das Kennwort ebenfalls.

## Schriftarten in der Datei

Eine Schriftart ist die Form der Buchstaben. Nicht jeder Computer hat jede Schriftart. Fehlt einem Computer eine Schriftart, die Ihre Folien verwenden, zeichnet er die Buchstaben in einer anderen Schrift. Dann können die Folien anders aussehen.

JeffDeck kann die Schriftarten, die Ihre Folien verwenden, in die Datei einbetten. Dann sehen die Folien auf jedem Computer gleich aus. Dies nennt man Einbetten. JeffDeck tut dies bei jedem Speichern. Um es abzuschalten, klicken Sie auf **Datei > Optionen**, öffnen die Registerkarte **Speichern** und deaktivieren **Schriftarten in der Datei einbetten**.

- JeffDeck bettet nur die Schriftarten ein, die mit ihm mitgeliefert werden, etwa Fraunces, Lato und Open Sans. Andere Computer haben diese nicht. Eine Schriftart, die auf jedem Computer vorhanden ist, etwa Calibri oder Arial, bettet es nie ein. Eine Schriftart, deren Lizenz das Kopieren in eine Datei untersagt, lässt es weg.
- JeffDeck bettet jede Ausprägung ein, die die Folien verwenden: normal, fett, kursiv und fett kursiv. Verwenden die Folien nur die normale Ausprägung, wird nur diese eingebettet.
- JeffDeck bettet jeden Buchstaben jeder Schriftart ein, nicht nur die verwendeten. Wer die Datei öffnet, kann deshalb die Wörter weiterhin ändern.
- Die Datei wird größer. Jede Ausprägung einer Schriftart fügt etwa 40 bis 700 KB hinzu (ein KB sind tausend Byte). Beim Speichern wird die Datei komprimiert, daher wächst sie um etwa die Hälfte davon.

Wenn Sie eine Datei mit eingebetteten Schriftarten öffnen, verwendet JeffDeck diese, solange die Präsentation geöffnet ist. Die Folien sehen so aus, wie sie erstellt wurden. JeffDeck installiert die Schriftarten nicht auf Ihrem Computer und kopiert sie nirgendwohin. Sie gehören nur zu dieser Präsentation: Sie erscheinen nicht in Ihren Schriftartenlisten, und keine andere Präsentation verwendet sie. Beim Schließen der Präsentation verschwinden sie. Hat Ihr Computer eine Schriftart bereits, verwendet JeffDeck Ihre eigene Kopie.

JeffDeck liest Schriftarten aus .pptx-Dateien, die andere Präsentationsprogramme erstellt haben, auch wenn das Programm sie komprimiert hat. Schriftarten aus .odp-Dateien liest es ebenfalls.

Eine Datei kann eine Schriftart enthalten, die JeffDeck nicht hat. Beim Speichern schreibt JeffDeck diese Schriftart wieder in die Datei, solange die Folien sie noch verwenden. Das geschieht unabhängig davon, ob **Schriftarten in der Datei einbetten** aktiviert ist. Ist eine Schriftart in einer Datei beschädigt oder nicht lesbar, sagt es die Meldung beim Öffnen der Datei.

## Dateiarten, die JeffDeck liest

- **.pptx, .potx und .ppsx** sind ein Format in drei Formen: Präsentation, Vorlage und Bildschirmpräsentation. JeffDeck liest und schreibt alle drei.
- **.ppt, .pps und .pot** sind die älteren Arten. JeffDeck liest sie, kann sie aber nicht schreiben. Daher fragt **Speichern** nach einem neuen Namen und speichert eine .pptx-Datei (bei .pps eine .ppsx-Datei). Das Original wird nie überschrieben.
- **.odp und .otp** sind OpenDocument-Präsentationen und ihre Vorlagen. JeffDeck liest und schreibt beide. Eine hier geöffnete .odp-Datei wird als .odp gespeichert. Eine .otp-Datei ist eine Vorlage, daher fragt **Speichern** nach einem neuen Namen und schlägt eine .odp-Datei vor.

Lässt sich ein Teil einer Datei nicht lesen, öffnet JeffDeck den Rest und listet auf, was es weggelassen hat. Lässt sich die ganze Datei nicht lesen, nennt es den Grund.

## Als .odp-Datei speichern

Eine .odp-Datei öffnet sich in LibreOffice Impress mit Ihren Folien, Mastern, Bildern, Notizen und Kommentaren sowie dem Erscheinungsbild, das JeffDeck anzeigt. Sie enthält Animationen, Folienübergänge, benutzerdefinierte Präsentationen und die Einstellungen für die Bildschirmpräsentation. Die Schriftart jedes Wortes wird mit ihrem Namen gespeichert, daher zeigt ein Computer ohne diese Schriftart eine andere an.

JeffDeck selbst liest Animationen, Kommentare, benutzerdefinierte Präsentationen und die Einstellungen der Bildschirmpräsentation derzeit noch nicht aus einer .odp-Datei zurück. Sie sind in der Datei enthalten, und LibreOffice spielt sie ab. Öffnen Sie die .odp-Datei aber in JeffDeck, listet das Programm sie als weggelassen auf. Behalten Sie Ihre .pptx-Datei, wenn Sie diese Elemente hier weiter bearbeiten wollen.

Eine .odp-Datei bietet für einige Dinge keinen Platz. JeffDeck listet sie nach dem Speichern in der Meldung unten im Fenster auf:

- **Diagramme, Diagramme und Gleichungen** werden als Bilder gespeichert. Sie können sie sehen, aber in der .odp-Datei nicht ihre Zahlen oder Teile bearbeiten.
- **Abschnitte** werden nicht gespeichert.
- **Abschrägungen und Innenschatten** werden weggelassen, ebenso Leuchten und Spiegelungen bei Wörtern. Ein Schatten bei Wörtern bleibt erhalten. Eine Spiegelung bei einer Form, eine Linie mit Farbverlauf, eine Linie aus zwei oder drei Strichen und die Größe eines Schattens bleiben in einer Form erhalten, die nur JeffDeck liest. Ein anderes Programm zeigt die Form ohne Spiegelung, die Linie in ihrer ersten Farbe, als einzelnen Strich und den Schatten in der Größe des Objekts.
- **Animationseffekte**, die beim Klick auf ein Objekt starten, und Effekte, die eine Farbe ändern, werden weggelassen. Ein Klick auf einen Film, um ihn anzuhalten oder erneut abzuspielen, bleibt erhalten.
- **Ein Film oder Sound, der aus einer anderen Datei verknüpft ist**, nicht in der Präsentation gespeichert, behält seine Adresse und sein Vorschaubild in der .odp-Datei. LibreOffice spielt ihn ab, wenn es die Datei findet. Filme und Sounds, die in der Präsentation enthalten sind, bleiben in der .odp-Datei erhalten und werden abgespielt.

Behalten Sie auch Ihre .pptx-Datei, wenn Sie diese Dinge zurückbekommen möchten.

## Was beibehalten, aber nicht angezeigt wird

Manche Teile einer Datei kann JeffDeck noch nicht anzeigen oder ändern. Es behält sie zusammen mit ihren Folien und schreibt sie beim Speichern zurück, sodass sie nicht verloren gehen. Dazu gehören Animationen, die in der Bildschirmpräsentation ablaufen, Filme und Sounds, die noch nicht abgespielt werden, sowie ein Übergangseffekt, für den JeffDeck keine Form hat und der als Überblendung abgespielt wird. Diagramme und Schaubilder werden so angezeigt, wie sie gespeichert wurden. Siehe [Willkommen bei JeffDeck](start).

## Siehe auch

- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Der Bereich „Datei“](file-screen)
- [Folien aus einer anderen Präsentation wiederverwenden](reuse)
- [Drucken, PDF und Folienbilder](printing)
