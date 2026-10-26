# Regeln für Zelleninhalte
<!-- keywords: Gültigkeit, Datenüberprüfung, Regel, Gültigkeitsregel, Liste, Dropdown, Dropdownliste, Auswahlliste, ganze Zahl, Dezimalzahl, Datum, Uhrzeit, Länge, Textlänge, benutzerdefiniert, Stopp, Warnung, Information, Fehlermeldung, nicht zulässig, Auswahl, Pfeil, Alt+Nach-unten -->

Eine Arbeitsmappe kann **Regeln** enthalten, die festlegen, was in bestimmte Zellen eingegeben werden darf: eine ganze Zahl von 1 bis 10, ein Datum nach einem bestimmten Tag, ein Text mit höchstens fünf Zeichen oder ein Eintrag aus einer Liste. Die Regeln stammen aus der Arbeitsmappe; JeffCrunch wendet sie an, wenn Sie einen Wert in eine Zelle **eingeben** (oder ihn aus ihrer Liste auswählen). Werte, die Sie einfügen, ausfüllen oder hineinverschieben, werden wie in gängigen Tabellenkalkulationen nicht geprüft. Eine Regel legt also fest, was eingegeben werden darf, nicht was die Zelle enthält.

## Wenn ein Wert nicht passt

Jede Regel hat eine **Art**, und die Meldung ist die der Regel, wenn die Arbeitsmappe eine eigene enthält:

- **Stopp**: Die Meldung wird angezeigt, und die Eingabe bleibt in der Zelle, damit Sie sie ändern können. Drücken Sie **Esc**, um sie zu verwerfen.
- **Warnung**: Sie werden gefragt, ob Sie den Wert trotzdem beibehalten möchten.
- **Information**: Sie werden darauf hingewiesen, dass der Wert nicht passt, und können fortfahren oder die Eingabe verwerfen.

Ein eingefügter Wert unterliegt der Regel nicht: Er wird unverändert übernommen, ohne Nachfrage. Das Leeren einer Zelle ist immer zulässig.

## Aus einer Liste auswählen

Eine Zelle mit einer Liste zeigt am rechten Rand eine kleine Schaltfläche mit einem Pfeil, wenn sich der Cursor in ihr befindet. Klicken Sie auf die Schaltfläche, oder drücken Sie **Alt+Nach-unten**, um die Auswahlmöglichkeiten anzuzeigen, und klicken Sie auf einen Eintrag, um ihn einzugeben.

## Was nicht unterstützt wird

JeffCrunch behält die Regeln der Arbeitsmappe bei, bietet aber noch keinen Befehl, um neue zu erstellen. Regeln, die ihre Auswahlmöglichkeiten in der neueren Schreibweise aus einem anderen Blatt lesen, bleiben in der Datei erhalten, werden aber nicht angewendet. Die Meldungen, die beim Auswählen einer Zelle erscheinen, werden nicht angezeigt.
