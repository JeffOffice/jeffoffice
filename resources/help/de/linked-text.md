# Text zwischen Textfeldern fließen lassen
<!-- keywords: Textfelder verknüpfen, verknüpfte Textfelder, Verknüpfung erstellen, Verknüpfung aufheben, Textfluss, Artikel fortsetzen, Überlauf, Text im Überlauf, Krug, Fortsetzung auf Seite, Fortsetzung von Seite, Textfelder verbinden, Kette, nächstes Textfeld, vorheriges Textfeld, automatischer Textfluss -->

Wenn ein Textabschnitt zu lang für ein Textfeld ist, können Sie das Textfeld mit einem zweiten verknüpfen. Der restliche Text fließt dann in das zweite Textfeld. Verknüpfte Textfelder bilden eine Kette, und der gesamte Text in einer Kette ist ein Textabschnitt. Wenn Sie in einem beliebigen Textfeld bearbeiten, fließen die Wörter erneut durch die ganze Kette.

## Zwei Textfelder verknüpfen

Das zweite Textfeld muss leer und darf noch nicht verknüpft sein. Zeichnen Sie es zuerst mit **Start > Objekte > Textfeld zeichnen**.

1. Klicken Sie auf das erste Textfeld, um es auszuwählen. Es muss das letzte Textfeld in seiner Kette sein.
2. Klicken Sie auf **Textfeld > Verknüpfen > Verknüpfung erstellen**. Der Mauszeiger wird zu einem Krug.
3. Klicken Sie auf das leere Textfeld. Der überzählige Text fließt hinein.

Sie können auch auf die rote Überlaufanzeige **A…** in der unteren rechten Ecke eines vollen Textfelds klicken. Damit erhalten Sie ebenfalls den Krug.

Wenn Sie stattdessen auf eine leere Stelle der Seite klicken, zeichnet JeffPub dort ein neues Textfeld in derselben Größe wie das erste und verknüpft es.

Das leere Textfeld kann sich auf einer anderen Seite befinden. Klicken Sie, während der Krug angezeigt wird, in der Seitenliste auf diese Seite und dann auf das Textfeld.

Wenn die Verknüpfung nicht gelingt, nennt die Statusleiste den Grund: Entweder enthält das angeklickte Textfeld bereits Text, oder es ist bereits verknüpft.

## So fließt der Text

Der Text füllt zuerst das erste Textfeld und dann das nächste in der Reihenfolge, in der Sie die Textfelder verknüpft haben. Die Reihenfolge hängt nicht davon ab, wo sich die Textfelder auf der Seite befinden. Hat ein Textfeld Spalten, werden zuerst die Spalten gefüllt. Ist das letzte Textfeld immer noch zu klein, erscheint die rote Anzeige **A…**, und der restliche Text bleibt ausgeblendet.

## Zwischen verknüpften Textfeldern wechseln

Wählen Sie ein verknüpftes Textfeld aus. Mit einem kleinen Pfeilfeld in der unteren rechten Ecke springen Sie zum nächsten Textfeld, mit einem in der oberen linken Ecke zum vorherigen. Sie können auch **Textfeld > Verknüpfen > Weiter** und **Textfeld > Verknüpfen > Zurück** verwenden. JeffPub wechselt dabei automatisch die Seite.

## Eine Verknüpfung aufheben

1. Wählen Sie das Textfeld direkt vor der Unterbrechung aus.
2. Klicken Sie auf **Textfeld > Verknüpfen > Verknüpfung aufheben**.

Der gesamte Text bleibt im ersten Teil der Kette. Die Textfelder nach der Unterbrechung werden leer, und Sie können sie erneut verknüpfen.

## Hinweise „Fortsetzung auf“ und „Fortsetzung von“ anzeigen

Ein Hinweis zeigt den Lesern, wo ein Textabschnitt weitergeht.

1. Klicken Sie mit der rechten Maustaste auf ein verknüpftes Textfeld, und wählen Sie **Objekt formatieren…**.
2. Öffnen Sie die Registerkarte **Textfeld**.
3. Aktivieren Sie **"Fortsetzung auf Seite…" einschließen**, um einen Hinweis am unteren Rand des Textfelds hinzuzufügen. Aktivieren Sie **"Fortsetzung von Seite…" einschließen**, um einen Hinweis am oberen Rand hinzuzufügen.

Der Hinweis lautet „(Fortsetzung auf Seite 4)“ oder „(Fortsetzung von Seite 2)“ und erscheint in kleiner kursiver Schrift. JeffPub trägt die Seitenzahl ein. Der erste Hinweis wird nur angezeigt, wenn ein Textfeld folgt, der zweite nur, wenn ein Textfeld vorausgeht.

## Seiten von JeffPub hinzufügen lassen

Wenn Sie **Einfügen > Text > Datei einfügen** verwenden und der Text zu lang ist, fragt JeffPub, ob Sie ihn auf neuen Seiten fortsetzen möchten. Wählen Sie **Ja**. JeffPub fügt Seiten mit je einem verknüpften Textfeld hinzu, an derselben Stelle und in derselben Größe wie das letzte Textfeld.

## Verwandte Themen

- [Textfelder hinzufügen und formatieren](text-boxes)
- [Die Registerkarte „Textfeld“](tab-text-box)
- [Seiten hinzufügen, verschieben, löschen und benennen](pages)
