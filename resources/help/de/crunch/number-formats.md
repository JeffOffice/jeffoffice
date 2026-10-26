# Zahlenformate und Ihre Sprache
<!-- keywords: Zahlenformat, Zahlen, Dezimalzahl, Dezimaltrennzeichen, Dezimalkomma, Dezimalstellen, Nachkommastellen, Tausender, Tausendertrennzeichen, Trennzeichen, Zifferngruppierung, Komma, Punkt, Sprache, Gebietsschema, Ländereinstellung, Währung, Prozent, Prozentsatz, Datum, Uhrzeit, Zeit, wissenschaftlich, Exponentialschreibweise, Bruch, Text, Buchhaltung, Standard, benutzerdefiniert, Formatcode, Code, Beispiel, Zellen formatieren, Strg+1, Strg+Umschalt+4, Strg+Umschalt+5, Dezimalstelle hinzufügen, Dezimalstelle entfernen, 1.000er-Trennzeichen, Formeltrennzeichen, Listentrennzeichen, Semikolon -->

Eine Zelle enthält eine Zahl, und ihr **Zahlenformat** legt fest, wie diese Zahl auf dem Bildschirm dargestellt wird. Das Format ändert nur, was Sie sehen, nie den Inhalt der Zelle. Beispiel: Die Zelle enthält 0,256. Mit dem Format „Prozent“ wird **25,6 %** angezeigt, mit „Währung“ **0,26 €** und mit „Standard“ **0,256**. Wenn Sie auf die Zelle klicken, zeigt die Bearbeitungsleiste weiterhin 0,256.

## Zahlen werden so geschrieben, wie Ihr Computer sie schreibt

JeffCrunch verwendet die Zeichen der Sprache, auf die Ihr Computer eingestellt ist. Im Englischen (USA) wird eintausendzweihundertvierunddreißigeinhalb als **1,234.5** geschrieben, im Deutschen als **1.234,5** und im Französischen als **1 234,5**. Die Arbeitsmappe ändert sich dabei nicht: Sie speichert eine neutrale Form, sodass dieselbe Datei einer Person 1,234.5 und einer anderen 1.234,5 anzeigt.

Das gilt überall, wo eine Zahl angezeigt oder eingegeben wird:

- Eine Zelle, die Kopie einer Zelle und die Breite, die **Spaltenbreite automatisch anpassen** einer Spalte gibt, richten sich nach den Zeichen Ihrer Sprache.
- Die **Bearbeitungsleiste** und eine Zelle, die Sie mit **F2** bearbeiten, übernehmen eine Zahl so, wie Sie sie eingeben. Wenn Ihre Sprache Dezimalzahlen mit Komma schreibt, geben Sie **2,5** ein, nicht 2.5.
- In einer Sprache, die Dezimalzahlen mit Komma schreibt, stehen zwischen den Argumenten einer Formel **Semikolons**: **=SUM(A1;A2)**. Die Funktionsnamen bleiben englisch.
- Datum und Uhrzeit richten sich nach dem Code des Formats: Ein Format, das „Monat, Tag, Jahr“ vorgibt, zeigt sie in dieser Reihenfolge an, und ein Format für Ihre Sprache (die vorgefertigten Formate in der Liste) verwendet die Reihenfolge und die Namen Ihrer Sprache.

Text, den das Format in Anführungszeichen setzt, etwa "Stück", wird unverändert angezeigt.

## Format auswählen

1. Markieren Sie die Zellen.
2. Verwenden Sie die Gruppe **Zahl** auf der Registerkarte **Start**, oder drücken Sie **Strg+1** (auf dem Mac **Befehl+1**), um **Zellen formatieren** zu öffnen. Das Dialogfeld öffnet sich mit der Registerkarte **Zahl**.

Das Feld in der Gruppe **Zahl** listet die gängigen Formate auf: Standard, Zahl, Währung, Buchhaltung, Datum, kurz, Datum, lang, Uhrzeit, Prozent, Bruch, Wissenschaftlich, Text und Benutzerdefiniert. **Weitere Zahlenformate** am Ende öffnet das Dialogfeld „Zellen formatieren“. Unter dem Feld befinden sich diese Schaltflächen:

| Schaltfläche | Funktion |
| --- | --- |
| Währung | Zeigt die Zahl als Geldbetrag mit dem Währungssymbol Ihrer Sprache an (**Strg+Umschalt+4**). Der Pfeil daneben bietet „Standard“ und **Weitere Zahlenformate** an. |
| Prozentformat | Multipliziert mit 100 und fügt ein Prozentzeichen ohne Dezimalstellen hinzu (**Strg+Umschalt+5**). Klicken Sie erneut darauf, um es zu entfernen. |
| 1.000er-Trennzeichen | Gruppiert die Ziffern nach Tausendern, zeigt zwei Dezimalstellen an und richtet die Dezimaltrennzeichen innerhalb einer Spalte untereinander aus, ohne Symbol. |
| Dezimalstelle hinzufügen, Dezimalstelle entfernen | Fügt eine Dezimalstelle hinzu oder entfernt eine. |
| Standard (im Feld und im Pfeil neben „Währung“) | Entfernt das Format: Die Zahl wird so angezeigt, wie sie ist (**Strg+Umschalt+~**). |

## Die Registerkarte „Zahl“ in „Zellen formatieren“

Wählen Sie links eine **Kategorie** aus. Rechts werden die zugehörigen Optionen angezeigt, und unter **Beispiel** sehen Sie, wie die erste markierte Zelle in Ihrer Sprache aussehen wird.

- **Zahl**: Dezimalstellen, ob die Ziffern nach Tausendern gruppiert werden und wie negative Zahlen aussehen (mit Minuszeichen, rot oder in Klammern).
- **Währung** und **Buchhaltung**: Dezimalstellen und ein Symbol. „Buchhaltung“ richtet die Symbole und die Dezimaltrennzeichen innerhalb einer Spalte untereinander aus.
- **Datum** und **Uhrzeit**: eine Liste vorgefertigter Formate mit der Reihenfolge und den Namen Ihrer Sprache.
- **Prozent**, **Wissenschaftlich**: Dezimalstellen.
- **Bruch**: vorgefertigte Formate wie „Einstellig“ (1/4), „Bis zu zwei Ziffern“ (21/25), „Als Viertel“ und „Als Sechzehntel“.
- **Sonderformat**: vorgefertigte Formate für eine Postleitzahl, Postleitzahl + 4, eine Telefonnummer und eine Sozialversicherungsnummer.
- **Text**: Die Zelle übernimmt Ihre Eingabe als Text, sodass eine Zahl wie 00123 ihre Nullen behält.
- **Benutzerdefiniert**: Geben Sie einen eigenen Code ein, oder beginnen Sie mit einem Code aus der Liste (die gängigen Codes und die, die bereits in der Arbeitsmappe verwendet werden).

Der **Code** ist der kurze Text, der JeffCrunch sagt, wie eine Zahl geschrieben wird. Beispiel: **#,##0.00** bedeutet „Ziffern nach Tausendern gruppieren und immer zwei Dezimalstellen anzeigen“, sodass 1234,5 in deutscher Schreibweise als 1.234,50 angezeigt wird. Jede **0** steht für eine Ziffer, die immer angezeigt wird, jedes **#** für eine Ziffer, die nur bei Bedarf angezeigt wird, und **%** multipliziert mit 100. Ein Code kann bis zu vier durch Semikolons getrennte Abschnitte haben: für positive Zahlen, negative Zahlen, Null und Text.

## Was gespeichert wird

Der Code wird in seiner neutralen Form in der Arbeitsmappe gespeichert, sodass die Arbeitsmappe auf jedem Computer richtig angezeigt wird. Das Formatieren ist ein einzelner Schritt, den Sie rückgängig machen können, und eine Arbeitsmappe, die ein Format bereits verwendet, behält es genau bei: Ein Format, das in der Liste steht, wird wiedergefunden, und keines wird doppelt angelegt.
