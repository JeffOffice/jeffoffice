# Spaltenbreite und Zeilenhöhe ändern
<!-- keywords: Standardbreite, Standardzeilenhöhe, Standardgröße, Breite, Höhe, Spalte, Zeile, Größe, breiter, höher, schmaler, niedriger, anpassen, automatisch anpassen, optimale Breite, ausblenden, einblenden, ziehen, Rahmen, Doppelklick, Zeichen, Punkt, Strg+0, Strg+9, Pixel, Zahl zu breit, Rautenzeichen, ### -->

Die **Breite einer Spalte** wird in Zeichen gemessen: wie viele Ziffern nebeneinander hineinpassen. Die **Höhe einer Zeile** wird in Punkt gemessen, einer Einheit aus der Typografie (72 Punkt ergeben ein Zoll). Eine normale Spalte fasst etwa 8 Ziffern, und eine normale Zeile ist 15 Punkt hoch.

## Rahmen ziehen

Zeigen Sie auf die Linie zwischen zwei Spaltenbuchstaben (oder zwei Zeilennummern). Der Mauszeiger wird zu Pfeilen, und Sie können die Linie ziehen. Eine gestrichelte Linie zeigt, wo der Rahmen landet; geändert wird erst, wenn Sie die Maustaste loslassen. Ziehen Sie die Linie ganz bis zum anderen Rahmen, um die Spalte oder Zeile auszublenden.

Wenn ganze Spalten (oder Zeilen) markiert sind und Sie den Rahmen einer davon ziehen, erhalten alle dieselbe Größe.

## An den Inhalt anpassen

**Doppelklicken** Sie auf den Rahmen, damit die Spalte genau so breit wird wie ihr längster Eintrag oder die Zeile genau so hoch wie ihre höchste Zeile. Das geht auch über **Start > Format > Spaltenbreite automatisch anpassen** und **Zeilenhöhe automatisch anpassen**, für alle markierten Spalten oder Zeilen auf einmal. Eine leere Spalte oder Zeile behält ihre Größe.

## Größe eingeben

Markieren Sie die Spalten oder Zeilen, wählen Sie **Start > Format > Spaltenbreite** oder **Zeilenhöhe**, und geben Sie die Größe ein. Mit der Größe 0 werden sie ausgeblendet.

## Standardgröße ändern

Spalten und Zeilen ohne eigene Größe erhalten die **Standardgröße**: 8,43 Zeichen breit und 15 Punkt hoch, sofern die Arbeitsmappe nichts anderes festlegt. Um sie für das ganze Blatt zu ändern, wählen Sie **Start > Format > Standardbreite** oder **Standardzeilenhöhe**, geben die Größe ein und klicken auf **OK**. Alle Spalten (oder Zeilen) ohne eigene Größe ändern sich sofort; die Spalten und Zeilen, deren Größe Sie selbst festgelegt haben, behalten sie. Dieser Schritt lässt sich rückgängig machen, und die Arbeitsmappe speichert den neuen Standard.

## Aus- und einblenden

**Strg+0** blendet die markierten Spalten aus, **Strg+9** die markierten Zeilen. Um ausgeblendete wieder anzuzeigen, markieren Sie die Spalten oder Zeilen auf beiden Seiten (oder einen Bereich, der sie umfasst), und wählen Sie **Start > Format > Spalten einblenden** oder **Zeilen einblenden**. Diese Befehle finden Sie auch im Kontextmenü der Spaltenbuchstaben und Zeilennummern.

## Eine Zahl, die zu breit für ihre Zelle ist

Wenn Sie eine Zahl eingeben oder einfügen, die nicht in ihre Spalte passt, wird die Spalte verbreitert, sofern sie noch ihre ursprüngliche Breite hat. Eine Spalte, deren Breite Sie selbst festgelegt haben, behält diese, und eine Zahl, die nicht hineinpasst, wird als **#**-Zeichen angezeigt, bis Sie die Spalte verbreitern. Eingabe und Verbreiterung sind ein einziger Schritt, **Rückgängig** nimmt also beides zurück.

Jede Änderung ist ein Schritt, den Sie rückgängig machen können, und die Datei speichert die Größen. Zur Orientierung: Eine Spalte mit 8,43 Zeichen Breite ist etwa 64 Pixel breit, eine Zeile mit 15 Punkt Höhe 20 Pixel hoch, jeweils bei 100 % Zoom.

## Zeilen, die mit ihrem Text wachsen

Eine Zeile, der Sie keine eigene Höhe gegeben haben, richtet sich nach ihrem Inhalt. Schalten Sie **Textumbruch** ein, und die Zeile wird so hoch, dass jede Textzeile sichtbar ist; vergrößern Sie die Schrift oder geben Sie mehrere Zeilen in eine Zelle ein (halten Sie **Alt** gedrückt und drücken Sie **Eingabe** für eine neue Zeile, wodurch auch der Textumbruch eingeschaltet wird), und sie wird entsprechend höher. Wenn der Text entfernt oder die Spalte verbreitert wird, wird die Zeile wieder niedriger. Eine Zeile, deren Höhe Sie selbst festgelegt oder die Sie ausgeblendet haben, bleibt unverändert. Diese Änderungen ergeben sich aus dem Zellinhalt und sind daher keine eigenen Schritte: Rückgängig ändert die Zellen, und die Zeile passt sich an. Ein sehr großes Blatt wird nur im sichtbaren Bereich gemessen; eine Zeile wird gemessen, sobald Sie zu ihr scrollen.
