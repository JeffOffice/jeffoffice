# Querverweise
<!-- keywords: Querverweis, Querverweise, Verweis, REF, PAGEREF, NOTEREF, siehe Seite, siehe Abbildung, oben, unten, Überschriftennummer, Link, Verknüpfung -->

Ein **Querverweis** ist Text in Ihrem Dokument, der auf eine andere Stelle verweist, etwa „siehe Abbildung 2 auf Seite 7“. Ein Querverweis ist ein Feld: Wenn sich die Abbildung oder die Seite ändert, aktualisieren Sie das Feld, und der Text passt sich an.

## Querverweis einfügen

1. Klicken Sie an die Stelle, an der der Text stehen soll.
2. Klicken Sie auf **Verweise > Querverweis**.
3. Wählen Sie den **Verweistyp**: eine Überschrift, eine Textmarke, eine Fußnote, eine Endnote, ein nummeriertes Element oder eine Beschriftung wie Abbildung oder Tabelle.
4. Wählen Sie unter **Verweisen auf**, was eingefügt werden soll: den Text, die Nummer, die Seitenzahl oder „oben“ bzw. „unten“.
5. Wählen Sie das Element in der Liste aus, lassen Sie **Als Link einfügen** aktiviert, wenn Sie dorthin springen möchten, und klicken Sie auf **Einfügen**.

Beispiel: Um „Siehe die Überschrift Ergebnisse auf Seite 4“ zu schreiben, geben Sie `Siehe die Überschrift ` ein, fügen einen Verweis auf die Überschrift mit **Überschriftentext** ein, geben ` auf Seite ` ein und fügen einen zweiten Verweis auf dieselbe Überschrift mit **Seitenzahl** ein.

## Querverweis folgen

Halten Sie **Strg** gedrückt, und klicken Sie auf einen Querverweis, der als Link eingefügt wurde. Die Einfügemarke springt an die genannte Stelle.

## Text aktuell halten

Wenn Sie Seiten hinzugefügt, Überschriften geändert oder Abbildungen verschoben haben, markieren Sie den Text, und drücken Sie **F9**. Alle Querverweise ermitteln ihren Text neu. Wurde die Stelle, auf die ein Querverweis zeigt, gelöscht, zeigt das Feld `Fehler: Die Textmarke, auf die verwiesen wird, fehlt.` an, und Sie können ein neues Ziel auswählen.

## Was im Hintergrund geschieht

JeffWrite kennzeichnet das Ziel mit einer ausgeblendeten Textmarke (ihr Name beginnt mit `_Ref`) und erstellt ein Feld, das darauf verweist: `REF` für Text und Nummern, `PAGEREF` für Seitenzahlen und `NOTEREF` für Fuß- und Endnoten. Auch in anderen Programmen funktionieren die Querverweise nach dem Öffnen der Datei weiterhin.
