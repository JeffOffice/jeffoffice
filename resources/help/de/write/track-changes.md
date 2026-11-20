# Änderungen nachverfolgen, annehmen und ablehnen
<!-- keywords: vergleichen, Dokumente vergleichen, zwei Versionen, was geändert wurde, Änderungen nachverfolgen, nachverfolgte Änderungen, Überarbeitungen, Markup, annehmen, ablehnen, alle annehmen, alle ablehnen, zur Überprüfung anzeigen, einfaches Markup, alle Markups, kein Markup, Original, Markup anzeigen, Sprechblasen, Überarbeitungsbereich, Änderungsbalken, Einfügung, Löschung, strg+umschalt+e, Autor, Bearbeiter, Benutzername, Bearbeitungen von anderen -->

Mit **Änderungen nachverfolgen** wird jede Bearbeitung festgehalten, sodass Sie oder andere Personen sehen können, was sich geändert hat, und entscheiden können, ob die Änderung übernommen wird. Verwenden Sie die Funktion, wenn Sie ein Dokument bearbeiten, das jemand anderem gehört, oder wenn jemand es nach Ihnen überprüfen soll.

## Änderungen nachverfolgen ein- und ausschalten

Klicken Sie auf **Überprüfen > Änderungen nachverfolgen**, oder drücken Sie **Strg+Umschalt+E**. Die Statusleiste am unteren Rand des Fensters zeigt an, ob die Funktion eingeschaltet ist. Ein Klick auf diese Anzeige schaltet sie um. Die Einstellung wird im Dokument gespeichert, sodass sie auch dann eingeschaltet bleibt, wenn das Dokument von einer anderen Person zurückkommt.

Während die Funktion eingeschaltet ist:

- **Was Sie eingeben**, wird als Einfügung markiert.
- **Was Sie löschen**, wird nicht entfernt. Es bleibt durchgestrichen im Text stehen, damit es wiederhergestellt werden kann. Eine Ausnahme bildet Text, den Sie selbst bei eingeschalteter Nachverfolgung eingegeben haben: Wenn Sie ihn löschen, wird er einfach entfernt, weil ihn niemand anderes gesehen hat.
- **Ein Absatzwechsel, den Sie machen** (Eingabetaste), wird als eingefügt markiert, und einer, den Sie löschen (Rücktaste am Absatzanfang), wird als gelöscht markiert.
- **Eine Änderung der Darstellung** (Fett, Schriftart, Schriftgröße, Farbe, Ausrichtung, Einzüge oder Formatvorlage) wird mit dem früheren Aussehen des Textes festgehalten.

Jede Änderung wird mit dem Namen der Person und dem Zeitpunkt gespeichert. Der Name ist der unter **Datei > Optionen** im Feld **Benutzername** eingetragene. Ändern Sie ihn dort, falls er falsch ist.

Jede Bearbeitung ist ein einzelner Schritt, daher nehmen Sie sie mit **Strg+Z** zurück, wie jede andere Bearbeitung auch.

## So sehen Änderungen aus

Standardmäßig hat jede Person eigene Farben für ihre Änderungen. Die erste Person, deren Änderung im Dokument angezeigt wird, erhält die erste Farbe, die nächste Person die zweite Farbe und so weiter. Eine Person behält ihre Farbe, solange das Dokument geöffnet ist. Eingefügter Text ist in der Farbe der Person unterstrichen, gelöschter Text ist durchgestrichen. Verschobener Text ist grün: an der Stelle, von der er entfernt wurde, doppelt durchgestrichen, und an der Stelle, an die er kam, doppelt unterstrichen. Text, dessen Formatierung geändert wurde, ist fett. Ein Balken am linken Rand markiert jede Zeile mit einer Änderung.

Sie können all dies ändern. Klicken Sie auf **Überprüfen > Nachverfolgung > Optionen für Nachverfolgung**, um das Dialogfeld **Optionen für Änderungsverfolgung** zu öffnen. Unter **Markup** hat jede Art von Änderung eine **Darstellung** und eine **Farbe**:

- **Einfügungen**: Unterstreichung, Doppelte Unterstreichung, Durchgestrichen, Fett, Kursiv, nur Farbe oder (ohne).
- **Löschungen**: Durchgestrichen, Doppelt durchgestrichen, Ausgeblendet, Caret oder nur Farbe. Bei **Ausgeblendet** werden gelöschte Wörter aus der Zeile weggelassen. Bei **Caret** werden sie ebenfalls weggelassen, und ein kleines ^ zeigt an, wo sie standen. Verschobener Text wird ebenfalls weggelassen, wenn Sie für Löschungen **Sprechblasen** wählen, bei anderen Einstellungen jedoch nicht.
- **Verschoben von** und **Verschoben nach**: die Darstellungen und Farben für verschobenen Text. Diese stehen zur Verfügung, solange **Verschiebungen nachverfolgen** aktiviert ist. Ist das Kontrollkästchen deaktiviert, wird verschobener Text als normale Löschung und normale Einfügung angezeigt.
- **Formatierung**: Fett, nur Farbe oder (ohne). Wenn Sie **Formatierung nachverfolgen** deaktivieren, werden Formatänderungen im Text nicht markiert.
- **Änderungsbalken**: linker Rahmen, rechter Rahmen, äußerer Rahmen (die rechte Seite auf ungeraden und die linke Seite auf geraden Seiten) oder (ohne) sowie die Farbe des Balkens.

Jedes Feld **Farbe** bietet **Nach Bearbeiter** (jede Person in ihrer eigenen Farbe) oder eine einzige Farbe für alle Personen. Diese Einstellungen gehören Ihnen. Sie werden auf diesem Computer gespeichert, nicht im Dokument, gelten für alle Dokumente und alle geöffneten Fenster, und jedes Fenster wird sofort aktualisiert, sobald Sie auf **OK** klicken. Die Balken werden auf dem Bildschirm, im Ausdruck und in einer PDF-Datei gleich gezeichnet, solange das Markup angezeigt wird.

## Auswählen, was Sie sehen: Zur Überprüfung anzeigen

Klicken Sie auf **Überprüfen > Zur Überprüfung anzeigen** und wählen Sie eine Option:

- **Alle Markups** zeigt jede Änderung im Text, mit den Balken am Rand.
- **Einfaches Markup** zeigt das Dokument so, wie es mit angenommenen Änderungen aussehen wird, und einen Balken am Rand bei jeder Zeile mit einer Änderung.
- **Kein Markup** zeigt das Dokument so, wie es mit allen angenommenen Änderungen aussehen wird, und sonst nichts.
- **Original** zeigt das Dokument so, wie es vor allen Änderungen aussah: Gelöschter Text ist wieder da, eingefügter Text ist verschwunden.

Diese Optionen ändern nur, was Sie sehen. Sie verändern das Dokument nicht und werden nicht mit ihm gespeichert. Ein Dokument mit Änderungen wird mit **Alle Markups** geöffnet.

**Überprüfen > Markup anzeigen** legt fest, welche Arten von Änderungen angezeigt werden: **Einfügungen und Löschungen**, **Formatierung** und **Kommentare**. Darunter wählen Sie, wie gelöschter Text dargestellt wird. **Alle Überarbeitungen inline anzeigen** lässt ihn im Text, durchgestrichen. **Überarbeitungen in Sprechblasen anzeigen** nimmt gelöschten Text aus der Zeile und zeigt ihn in einer Sprechblase am Rand. **Nur Kommentare und Formatierungen in Sprechblasen anzeigen** lässt Einfügungen und Löschungen im Text.

## Änderungen annehmen oder ablehnen

Eine Änderung anzunehmen heißt, sie endgültig zu übernehmen. Eine Änderung abzulehnen heißt, sie rückgängig zu machen.

- **Überprüfen > Annehmen** nimmt die Änderung an, in der sich die Einfügemarke befindet (oder jede Änderung im markierten Text), und springt dann zur nächsten. **Überprüfen > Ablehnen** tut dasselbe und macht die Änderung rückgängig.
- Klicken Sie auf den Pfeil unter **Annehmen** oder **Ablehnen** für weitere Möglichkeiten: **Alle angezeigten Änderungen annehmen**, **Alle Änderungen annehmen** und **Alle Änderungen annehmen und Nachverfolgung beenden**; sowie **Alle angezeigten Änderungen ablehnen** und **Alle Änderungen ablehnen**. „Angezeigt“ bedeutet die Änderungen der Arten, die unter **Markup anzeigen** eingeschaltet sind (und im Bereich **Überarbeitungsbereich** den ausgewählten Bearbeiter).
- **Zurück** und **Weiter** in der Gruppe **Änderungen** führen von einer Änderung zur nächsten, ohne etwas zu verändern.

Jeder dieser Befehle ist ein einzelner Schritt, daher bringt **Strg+Z** das Dokument genau in den Zustand zurück, in dem es vorher war.

Das Annehmen einer Löschung entfernt den Text endgültig. Das Ablehnen einer Einfügung entfernt den Text ebenfalls. Das Annehmen einer Einfügung behält den Text und entfernt nur die Markierung, dass er neu ist. Das Ablehnen einer Löschung behält den Text und entfernt die Durchstreichung. Das Ablehnen einer Änderung der Darstellung stellt das frühere Aussehen des Textes wieder her. Verschobener Text besteht aus einem Paar von Änderungen, einer an der Stelle, von der er entfernt wurde, und einer an der Stelle, an die er kam. Beide sind durch den Namen der Verschiebung verbunden. Wenn das Paar mit demselben Namen markiert ist, wie andere Programme es speichern, wirkt das Annehmen oder Ablehnen einer Hälfte auf die andere Hälfte, sodass der Text endgültig verschoben oder an seine ursprüngliche Stelle zurückgesetzt wird. Ein Name, der mehrfach verwendet wird oder nur eine Hälfte hat, verbindet nichts: Jede Hälfte ist dann eine eigenständige Änderung.

## Der Überarbeitungsbereich

Klicken Sie auf **Überprüfen > Überarbeitungsbereich**, um eine Liste der Änderungen im Dokument zu öffnen: Art, Bearbeiter, Zeitpunkt und die Wörter. Ein Klick auf einen Eintrag zeigt die Änderung im Text. Verwenden Sie unter der Liste **Annehmen**, **Ablehnen**, **Zurück** und **Weiter**, oder wählen Sie im Feld darüber einen Bearbeiter aus, um nur dessen Änderungen zu sehen.

## Änderungen in Kopfzeilen, Fußzeilen, Notizen und Tabellen

Änderungen, die andere in einer Kopfzeile, einer Fußzeile, einer Fußnote oder einer Endnote vorgenommen haben, werden wie die Änderungen im Text angezeigt und aufgelistet. Das gilt auch für eine Kopf- oder Fußzeile, die keine Seite verwendet (etwa die für gerade Seiten, wenn das Dokument keine unterschiedlichen geraden und ungeraden Seiten verwendet). **Weiter** geht an ihr vorbei, und **Annehmen** und **Ablehnen** führen die Änderung aus, bei der Sie dort zuletzt angehalten haben. **Weiter**, **Zurück**, **Annehmen**, **Ablehnen** und der **Überarbeitungsbereich** führen Sie in die Kopfzeile, Fußzeile oder Notiz mit der Änderung. Die Reihenfolge ist: zuerst der Text, dann die Fußnoten, dann die Kopf- und Fußzeilen. **Alle annehmen** und **Alle ablehnen** gelten für alle diese Bereiche.

Auch Änderungen an Tabellen werden aufgelistet, im Text sowie in Kopf- und Fußzeilen: eine eingefügte oder gelöschte Zeile oder Zelle, verbundene Zellen und eine Änderung der Darstellung einer Tabelle, einer Zeile oder einer Zelle. Ein Balken am Rand markiert die Zeilen der Zeile oder Zelle. Das Annehmen einer gelöschten Zeile oder das Ablehnen einer eingefügten Zeile entfernt die Zeile und ihren Inhalt. Eine Zeile, in der jede Zelle die Änderung hat, gilt als Änderung dieser Zeile. Das Ablehnen einer Änderung der Darstellung stellt das frühere Aussehen der Tabelle, der Zeile oder der Zelle wieder her, und das Ablehnen einer Verbindung teilt die Zellen wieder. Würde das Entfernen einer Änderung die Tabelle beschädigen, bleibt die Änderung stattdessen erhalten, und die Meldung gibt an, wie viele Änderungen beibehalten wurden. Das betrifft eine Zelle, die nach unten mit anderen verbunden ist: Sie bleibt und verliert ihre Markierung. Ebenso betrifft es eine Änderung am Raster, der die Zeilen nicht folgen können: Sie bleibt als Änderung, die Sie annehmen können. Änderungen innerhalb einer Tabelle in einer Fußnote bleiben so, wie sie geschrieben wurden: Sie werden nicht aufgelistet, und **Annehmen** und **Ablehnen** wirken nicht auf sie.

Eine Verschiebung ist eine Zeile in der Liste und eine Änderung in der Anzahl, die **Alle annehmen** und **Alle ablehnen** angeben: Ihre andere Hälfte und die Absatzmarken, die mit ihr gingen, sind damit erledigt. Änderungen innerhalb von Textfeldern werden nicht einzeln aufgelistet. Der **Überarbeitungsbereich** zeigt an, wie viele es sind (zum Beispiel „3 weitere Änderungen befinden sich in Textfeldern“), und **Alle annehmen** und **Alle ablehnen** erledigen sie zusammen mit den übrigen.

## Was noch nicht nachverfolgt wird

Änderungen, die Sie selbst an Tabellen (Zeilen, Zellen, Verbinden), an Nummerierungen und Listen, an Abschnitten und am Seitenlayout sowie am Text in Kopf- und Fußzeilen und Notizen vornehmen, werden wie üblich ausgeführt, ohne dass sie festgehalten werden. Änderungen, die andere Programme bei Nummerierungen und Abschnitten aufgezeichnet haben, bleiben so erhalten, wie sie waren, und werden auch so gespeichert. Sie werden aber nicht aufgelistet, und **Annehmen** und **Ablehnen** wirken nicht auf sie.

## Zwei Versionen vergleichen

Mit **Vergleichen** finden Sie die Unterschiede zwischen zwei Versionen eines Dokuments und zeigen sie als nachverfolgte Änderungen an.

1. Öffnen Sie die ältere Version. Sie ist das **Original**.
2. Klicken Sie in der Gruppe **Vergleichen** auf der Registerkarte **Überprüfen** auf **Vergleichen**.
3. Klicken Sie auf **Durchsuchen** und wählen Sie die neuere Version. Sie ist die **überarbeitete** Version.
4. Geben Sie den Namen ein, mit dem die Änderungen gekennzeichnet werden sollen. Wählen Sie die gewünschten Optionen, etwa das Ignorieren von Groß- und Kleinschreibung. **Verschobenen Text suchen** und **Formatänderungen suchen** sind eingeschaltet, sofern Sie sie nicht deaktivieren.
5. Klicken Sie auf **OK**.

Es öffnet sich ein neues Fenster mit dem ursprünglichen Text und den Änderungen, die ihn in den überarbeiteten Text überführen. Ihre beiden Dateien bleiben unverändert. Verwenden Sie **Annehmen** und **Ablehnen** wie bei jeder anderen nachverfolgten Änderung. Wenn Sie alle Änderungen annehmen, erhalten Sie den überarbeiteten Text. Wenn Sie alle Änderungen ablehnen, erhalten Sie den ursprünglichen Text.

Der Vergleich betrachtet den Text im Hauptteil des Dokuments. Tabellen, Kopfzeilen, Fußzeilen und Fußnoten werden nicht berücksichtigt.

Ein Absatz mit 12 Wörtern oder mehr, der an einer Stelle herausgeschnitten und an einer anderen eingefügt wurde, wird als **Verschiebung** angezeigt: ein Eintrag an der Stelle, von der der Text entfernt wurde, und einer an der Stelle, an die er kam. Das Annehmen oder Ablehnen eines der beiden Einträge betrifft beide. Der Absatz muss identisch sein oder mindestens 9 von 10 Wörtern müssen übereinstimmen, und zwar überwiegend in derselben Reihenfolge. Andernfalls wird er als Löschung und Einfügung angezeigt. Wörter, die sich innerhalb eines verschobenen Absatzes unterscheiden, werden als Einfügungen und Löschungen innerhalb der Verschiebung angezeigt.

Text, der in beiden Versionen gleich ist, aber anders aussieht, wird als **Formatänderung** angezeigt. Der Vergleich prüft Fett, Kursiv, Unterstreichung, Schriftgröße, Schriftart, Farbe, Hervorhebung sowie Formatvorlage, Ausrichtung, Einzüge und Abstände eines Absatzes. Das Ablehnen der Änderung stellt das alte Aussehen wieder her.

Wenn die beiden Dokumente zu viele Änderungen haben, meldet der Vergleich dies. Er zeigt dann einige Absätze als vollständig ersetzt an und sucht möglicherweise nicht in allen nach Verschiebungen oder Formatänderungen.
