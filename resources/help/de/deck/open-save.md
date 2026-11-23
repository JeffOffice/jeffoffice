# Präsentationen öffnen und speichern
<!-- keywords: Makros, mit Makros, pptm, potm, ppsm, öffnen, speichern, speichern unter, otp, LibreOffice, weggelassen, schließen, neu, zuletzt verwendet, angeheftet, Dateitypen, pptx, potx, ppsx, ppt, odp, Vorlage, Show, umbenennen, Strg+S, Sicherung, Sicherungskopie, Dateiformate, alte Dateien, OpenDocument-Präsentation, erhalten, nicht angezeigt, Schriftarten, Schriftarten einbetten, eingebettete Schriftarten, fehlende Schriftart, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie erstellen, Meine Vorlagen, als Vorlage speichern, reparieren, öffnen und reparieren, beschädigt -->

JeffDeck speichert Ihre Arbeit in .pptx-Dateien und in .odp-Dateien und öffnet mehrere Arten von Präsentationsdateien.

## Eine neue Präsentation beginnen

Drücken Sie Strg+N. Ein neues Fenster mit einer leeren Präsentation wird geöffnet. Oder klicken Sie auf **Datei > Neu** und dann auf **Leere Präsentation**. Wenn das Fenster, in dem Sie gerade arbeiten, eine neue Präsentation enthält, die noch niemand bearbeitet hat, ersetzt die leere Präsentation sie. Andernfalls wird sie in einem eigenen Fenster geöffnet.

## Speichern

Drücken Sie Strg+S, oder klicken Sie auf **Datei > Speichern**. Beim ersten Mal fragt JeffDeck nach einem Namen und einem Speicherort. Danach ersetzt Speichern die Datei durch Ihre neueste Fassung. Eine Meldung unten zeigt „Gespeichert“ und den Namen der Datei.

Ein Stern (*) nach dem Namen in der Titelleiste bedeutet, dass Sie Änderungen haben, die nicht gespeichert sind. Speichern Sie häufig. Wenn JeffDeck unerwartet beendet wird, lesen Sie [Nicht gespeicherte Arbeit wiederherstellen](recover).

Wenn eine Meldung besagt, dass einige Objekte „noch nicht in dieser Dateiart gespeichert werden können“, wird die Datei zwar gespeichert, JeffDeck konnte diese Objekte aber nicht schreiben. Behalten Sie auch die ursprüngliche Datei. Wenn Sie eine .odp-Datei speichern, listet die Meldung auf, was diese Dateiart nicht aufnehmen kann. Der Abschnitt „Als .odp-Datei speichern“ weiter unten beschreibt, was sie nicht aufnehmen kann.

## Mit neuem Namen oder in einer anderen Art speichern

Klicken Sie auf **Datei > Speichern unter**, oder drücken Sie F12. Geben Sie einen Namen ein, wählen Sie die Art, und klicken Sie auf **Speichern**. Die Auswahl umfasst:

- **Präsentation (*.pptx)** ist die übliche Wahl.
- **Vorlage (*.potx)** speichert die Folien als Muster, aus dem neue Präsentationen erstellt werden.
- **Bildschirmpräsentation (*.ppsx)** speichert eine Präsentation, die abgespielt werden soll.
- **Präsentation mit Makros (*.pptm)**, **Vorlage mit Makros (*.potm)** und **Bildschirmpräsentation mit Makros (*.ppsm)** sind dieselben drei Arten für eine Präsentation, die **Makros** enthält. Ein Makro ist ein kleines Programm, das mit einer Datei mitgeliefert wird. JeffDeck führt Makros nie aus und liest oder zeigt sie auch nicht an. Wenn Sie eine dieser Dateien öffnen und in derselben Art speichern, bleiben die Makros unverändert erhalten. Wenn Sie sie als .pptx, .potx oder .ppsx speichern, werden die Makros weggelassen, und JeffDeck weist Sie darauf hin. Eine Präsentation ohne Makros kann ebenfalls in diesen Arten gespeichert werden, hat aber keine Makros, die erhalten bleiben könnten. Sind die Makros in einer Datei zu groß, um sie zu behalten (mehr als 32 MB in einem Teil oder 64 MB insgesamt), lässt JeffDeck sie weg und teilt Ihnen beim Öffnen mit, dass dies geschehen ist.
- **OpenDocument-Präsentation (*.odp)** speichert die Folien in dem Dateityp, den LibreOffice und andere kostenlose Programme verwenden.
- **OpenDocument-Präsentationsvorlage (*.otp)** ist die Vorlagenform dieser Art.

Wenn Sie die Endung weglassen, fügt JeffDeck die zur Art passende Endung hinzu. Nach dem Speichern zeigt die Titelleiste den neuen Namen, und Sie arbeiten in der neuen Datei weiter.

Eine Präsentation, die Sie aus einer Vorlage geöffnet haben, wird als neue Präsentation gespeichert: **Speichern** fragt nach einem Namen, damit die Vorlage unverändert bleibt. Dasselbe gilt für eine Präsentation, die Sie aus einer älteren .ppt-, .pps- oder .pot-Datei geöffnet haben. Eine Präsentation, die Sie aus einer .odp-Datei geöffnet haben, wird wieder als .odp-Datei gespeichert, und zwar über dieselbe Datei.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**, und wählen Sie einen Namen. JeffDeck schreibt die Präsentation in diese Datei, und Sie arbeiten in der bisherigen weiter. Ihr Name und Ihre nicht gespeicherten Änderungen bleiben unverändert.

## Sicherungskopie behalten

Klicken Sie auf **Datei > Optionen**, und schalten Sie auf der Registerkarte **Allgemein** die Option **Immer Sicherungskopie erstellen** ein. Bei jedem Speichern kopiert JeffDeck zuerst die alte Datei in eine Datei mit dem Präfix „Sicherung von“ und dem Namen, im selben Ordner. Kann die Kopie nicht erstellt werden, fragt das Programm, ob ohne Sicherung gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja sagen, und die Datei bleibt unverändert.

## Als Vorlage speichern

Eine Vorlage ist eine Präsentation, die Sie als Ausgangspunkt verwenden. Klicken Sie auf **Datei > Als Vorlage speichern**, und geben Sie einen Namen ein. Die Vorlage (eine **.potx**-Datei) landet unter **Meine Vorlagen**, Ihre Präsentation bleibt unverändert. Um eine neue Präsentation aus der Vorlage zu beginnen, klicken Sie auf **Datei > Neu** und dann auf die Vorlage unter **Meine Vorlagen**. Die neue Präsentation hat noch keine eigene Datei, daher bleibt die Vorlage unverändert. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Öffnen

Drücken Sie Strg+O, um sofort eine Datei auszuwählen. Oder klicken Sie auf **Datei > Öffnen** und dann auf **Durchsuchen** oder auf einen Namen unter **Angeheftet** oder **Zuletzt verwendete Präsentationen**.

Die Datei wird in diesem Fenster geöffnet, wenn es eine neue Präsentation enthält, die noch niemand bearbeitet hat. Andernfalls wird sie in einem eigenen Fenster geöffnet.

Klicken Sie in der Liste mit der rechten Maustaste auf einen Namen, um ihn **An Liste anheften** (er bleibt oben), **Von Liste lösen** oder **Aus Liste entfernen**. Ein grau dargestellter Name bedeutet, dass JeffDeck die Datei nicht finden kann.

## Andere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch in der Liste der zuletzt verwendeten Dateien mit der rechten Maustaste auf einen Namen klicken.

- **Schreibgeschützt öffnen** öffnet die Präsentation nur zum Lesen. Sie können die Folien ansehen, durch sie blättern und kopieren, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** drücken, fragt JeffDeck nach einem neuen Namen, damit die geöffnete Datei unverändert bleibt.
- **Als Kopie öffnen** öffnet die Folien als neue Präsentation ohne eigene Datei. **Speichern** fragt, wohin sie gespeichert werden soll.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** ist für eine beschädigte .pptx- oder .ppt-Datei gedacht, zum Beispiel eine, die beim Kopieren abgeschnitten wurde. JeffDeck rettet so viel wie möglich: Es liest jeden Teil, den es lesen kann, behält einen Teil, der mitten im Vorgang abbrach, bis zu der Stelle, an der er abbrach, und lässt einen Teil weg, den es nicht lesen kann. Bei einer alten .ppt-Datei findet das Programm die Folien anhand ihres Inhalts, wenn die Listen, die angeben, wo sie gespeichert sind, beschädigt oder verloren sind. Wenn die Listen, die angeben, wie die Teile der Datei zusammenpassen, beschädigt sind, übernimmt das Programm jeden Teil als Fortsetzung des vorherigen und zeigt an, was es dort findet. Dieser Teil stammt möglicherweise von einer älteren Fassung der Präsentation oder von einem anderen Teil der Datei. Deshalb nennt die Meldung die Folie, an der er beginnt, und bittet Sie, ihn mit einer Sicherungskopie zu vergleichen, falls Sie eine haben. Liegt die Vermutung vor der ersten Folie, meldet die Meldung, dass die ganze Präsentation zusammengesetzt wurde. Konnten Bilder nicht vollständig wiederhergestellt werden, sagt die Meldung das ebenfalls. Ist die Liste verloren, die die Reihenfolge der Folien angibt, erscheinen die Folien in der Reihenfolge, in der sie gespeichert wurden, was nicht unbedingt ihre frühere Reihenfolge ist. Eine Datei mit Kennwort lässt sich nicht reparieren. Die reparierten Folien werden als Kopie mit dem Namen „Repariert“ und dem Dateinamen geöffnet, und eine Meldung teilt Ihnen mit, was getan wurde. Die beschädigte Datei bleibt unverändert.

## Dateien mit Kennwort

Eine **.pptx**- oder **.odp**-Datei, die ein Kennwort verlangt, wird wie gewohnt geöffnet: JeffDeck fragt nach dem Kennwort, und Sie können es erneut versuchen, wenn es nicht stimmt, oder auf **Abbrechen** klicken. Die Datei bleibt beim Speichern geschützt. Um das Kennwort zu ändern oder zu entfernen, verwenden Sie **Datei > Info > Mit Kennwort verschlüsseln** (siehe Seite „Datei“). Eine als .odp-Datei gespeicherte Kopie behält das Kennwort ebenfalls.

## Schriftarten in der Datei

Eine Schriftart ist die Form der Buchstaben. Nicht jeder Computer hat jede Schriftart. Fehlt einem Computer eine Schriftart, die Ihre Folien verwenden, zeichnet er die Buchstaben in einer anderen Schriftart. Dann können die Folien anders aussehen.

JeffDeck kann die Schriftarten, die Ihre Folien verwenden, in der Datei speichern. Dann sehen die Folien auf jedem Computer gleich aus. Das nennt man Einbetten. JeffDeck tut dies bei jedem Speichern. Um es abzuschalten, klicken Sie auf **Datei > Optionen**, öffnen die Registerkarte **Speichern** und deaktivieren **Schriftarten in der Datei einbetten**.

- JeffDeck nimmt nur die Schriftarten auf, die mit ihm mitgeliefert werden, etwa Fraunces, Lato und Open Sans. Andere Computer haben diese nicht. Es nimmt nie eine Schriftart auf, die jeder Computer hat, etwa Calibri oder Arial. Eine Schriftart, deren Lizenz das Kopieren in eine Datei untersagt, lässt es weg.
- JeffDeck nimmt jeden Schnitt auf, den die Folien verwenden: normal, fett, kursiv und fett kursiv. Verwenden die Folien nur den normalen Schnitt, wird nur dieser aufgenommen.
- JeffDeck nimmt jedes Zeichen jeder Schriftart auf, nicht nur die, die Sie verwendet haben. So kann eine Person, die die Datei öffnet, die Wörter trotzdem ändern.
- Die Datei wird größer. Jeder Schnitt einer Schriftart fügt etwa 40 bis 700 KB hinzu (ein KB sind tausend Byte). Die Datei wird beim Speichern komprimiert, daher wächst sie um etwa die Hälfte davon.

Wenn Sie eine Datei öffnen, die Schriftarten enthält, verwendet JeffDeck sie, solange die Präsentation geöffnet ist. Die Folien sehen so aus, wie sie dort erstellt wurden. JeffDeck installiert die Schriftarten nicht auf Ihrem Computer und kopiert sie auch nirgendwohin. Sie gehören nur zu dieser Präsentation: Sie erscheinen nicht in Ihren Schriftartenlisten, und keine andere Präsentation verwendet sie. Beim Schließen der Präsentation verschwinden sie. Hat Ihr Computer eine Schriftart bereits, verwendet JeffDeck Ihre eigene Kopie.

JeffDeck liest Schriftarten aus .pptx-Dateien, die mit anderen Präsentationsprogrammen erstellt wurden, auch wenn das Programm sie komprimiert hat. Auch aus .odp-Dateien liest es Schriftarten.

Eine Datei kann eine Schriftart enthalten, die JeffDeck nicht hat. Beim Speichern schreibt JeffDeck diese Schriftart wieder in die Datei, solange die Folien sie noch verwenden. Das geschieht unabhängig davon, ob **Schriftarten in der Datei einbetten** aktiviert ist. Ist eine Schriftart in einer Datei beschädigt oder nicht lesbar, sagt die Meldung beim Öffnen der Datei das.

## Arten von Dateien, die JeffDeck liest

- **.pptx, .potx und .ppsx** sind ein Format in drei Formen: Präsentation, Vorlage und Bildschirmpräsentation. JeffDeck liest und schreibt alle drei.
- **.ppt, .pps und .pot** sind die älteren Arten. JeffDeck liest sie, kann sie aber nicht schreiben. Daher fragt **Speichern** nach einem neuen Namen und speichert eine .pptx-Datei (bei .pps eine .ppsx-Datei). Die Originaldatei wird nie überschrieben.
- **.odp und .otp** sind OpenDocument-Präsentationen und ihre Vorlagen. JeffDeck liest und schreibt beide. Eine hier geöffnete .odp-Datei wird als .odp gespeichert. Eine .otp-Datei ist eine Vorlage, daher fragt **Speichern** nach einem neuen Namen und schlägt eine .odp-Datei vor.

Kann ein Teil einer Datei nicht gelesen werden, öffnet JeffDeck den Rest und listet auf, was es ausgelassen hat. Kann die ganze Datei nicht gelesen werden, sagt es, warum.

## Als .odp-Datei speichern

Eine .odp-Datei wird in LibreOffice Impress mit Ihren Folien, Masterfolien, Bildern, Notizen und Kommentaren geöffnet, und mit dem Erscheinungsbild, das JeffDeck anzeigt. Sie enthält auch Animationen, Folienübergänge, zielgruppenorientierte Präsentationen und die Einstellungen für die Bildschirmpräsentation. Die Schriftart jedes Wortes wird mit ihrem Namen gespeichert, daher zeigt ein Computer ohne diese Schriftart eine andere an.

JeffDeck selbst liest Animationen, Kommentare, zielgruppenorientierte Präsentationen oder die Einstellungen der Bildschirmpräsentation aus einer .odp-Datei noch nicht zurück. Sie sind in der Datei enthalten, und LibreOffice spielt sie ab. Wenn Sie die .odp-Datei in JeffDeck öffnen, listet das Programm sie aber als ausgelassen auf. Behalten Sie Ihre .pptx-Datei, wenn Sie diese Dinge hier weiter bearbeiten wollen.

Eine .odp-Datei bietet keinen Platz für einige Dinge. JeffDeck listet sie nach dem Speichern in der Meldung unten im Fenster auf:

- **Diagramme, Diagramme (SmartArt) und Formeln** werden als Bilder gespeichert. Sie können sie sehen, aber Ihre Zahlen oder Teile in der .odp-Datei nicht bearbeiten.
- **Abschnitte** werden nicht gespeichert.
- **Abschrägungen und innere Schatten** werden weggelassen, ebenso Leuchten und Spiegelungen bei Wörtern. Ein Schatten auf Wörtern bleibt erhalten. Eine Spiegelung auf einer Form, eine Linie mit Farbverlauf, eine Linie aus zwei oder drei Strichen und die Größe eines Schattens bleiben in einer Form erhalten, die nur JeffDeck liest. Andere Programme zeigen die Form ohne Spiegelung, die Linie in ihrer ersten Farbe, einen einzelnen Strich und den Schatten in der Größe des Objekts.
- **Animationseffekte**, die beginnen, wenn Sie auf ein Objekt klicken, und Effekte, die eine Farbe ändern, werden weggelassen. Ein Klick auf einen Film, um ihn anzuhalten oder erneut abzuspielen, bleibt erhalten.
- **Ein verknüpfter Film oder Ton** aus einer anderen Datei, der nicht in der Präsentation gespeichert ist, behält seine Adresse und sein Vorschaubild in der .odp-Datei. LibreOffice spielt ihn ab, wenn es die Datei finden kann. Filme und Töne, die in der Präsentation enthalten sind, bleiben in der .odp-Datei erhalten und werden abgespielt.

Behalten Sie auch Ihre .pptx-Datei, falls Sie diese Dinge zurückbenötigen.

## Was erhalten, aber nicht angezeigt wird

Einige Teile einer Datei kann JeffDeck noch nicht anzeigen oder ändern. Es behält sie zusammen mit ihren Folien und schreibt sie beim Speichern zurück, sodass sie nicht verloren gehen. Dazu gehören Animationen, die in der Bildschirmpräsentation abgespielt werden, Filme und Töne, die noch nicht wiedergegeben werden, und ein Übergangseffekt, für den JeffDeck keine Form hat und der als Verblassen abgespielt wird. Diagramme und Schaubilder werden so angezeigt, wie sie gespeichert wurden. Siehe [Willkommen bei JeffDeck](start).

## Siehe auch

- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Die Seite „Datei“](file-screen)
- [Folien aus einer anderen Präsentation wiederverwenden](reuse)
- [Drucken, PDF und Folienbilder](printing)
