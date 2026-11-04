# Zahlenformate und Ihre Sprache
<!-- keywords: Zahlenformat, Zahlen, Dezimalstelle, Dezimaltrennzeichen, Nachkommastellen, Tausender, Tausendertrennzeichen, Gruppierung, Komma, Punkt, Sprache, Gebietsschema, Regionaleinstellung, Währung, Prozent, Prozentsatz, Datum, Uhrzeit, wissenschaftlich, Bruch, Text, Buchhaltung, Standard, benutzerdefiniert, Code, Beispiel, Zellen formatieren, Strg+1, Strg+Umschalt+4, Strg+Umschalt+5, Dezimalstelle hinzufügen, Dezimalstelle entfernen, 1000er-Trennzeichen, Formeltrennzeichen, Semikolon -->

Eine Zelle enthält eine Zahl, und ihr **Zahlenformat** legt fest, wie diese Zahl auf dem Bildschirm geschrieben wird. Das Format ändert, was Sie sehen, aber nie, was die Zelle enthält. Beispiel: Die Zelle enthält 0,256. Mit dem Format Prozent wird **25,6 %** angezeigt, mit Währung **0,26 €** und mit Standard **0,256**. Klicken Sie auf die Zelle, dann steht in der Bearbeitungsleiste weiterhin 0,256.

## Zahlen werden so geschrieben, wie Ihr Computer sie schreibt

JeffCrunch verwendet die Zeichen der Sprache, auf die Ihr Computer eingestellt ist. In Deutsch ist eintausendzweihundertvierunddreißig und eine halbe Einheit **1.234,5**. In Englisch (USA) lautet dieselbe Zahl **1,234.5**, und auf Französisch **1 234,5**. Die Arbeitsmappe ändert sich dabei nicht: Sie speichert eine neutrale Form, sodass dieselbe Datei für die eine Person 1.234,5 und für die andere 1,234.5 anzeigt.

Das gilt überall, wo eine Zahl angezeigt oder eingegeben wird:

- Eine Zelle, die Kopie einer Zelle und die Breite, die **Spaltenbreite anpassen** einer Spalte gibt, richten sich nach den Zeichen Ihrer Sprache.
- Die **Bearbeitungsleiste** und eine Zelle, die Sie mit **F2** bearbeiten, zeigen eine Zahl so an, wie Sie sie eingeben. Wenn Ihre Sprache Dezimalstellen mit einem Komma schreibt, geben Sie **2,5** ein, nicht 2.5.
- In einer Sprache, die Dezimalstellen mit einem Komma schreibt, stehen in einer Formel zwischen den Argumenten **Semikolons**: **=SUMME(A1;A2)**. Die Namen der Funktionen bleiben auf Englisch.
- Datums- und Uhrzeitangaben folgen dem Code des Formats: Ein Format, das „Monat, Tag, Jahr“ vorsieht, zeigt sie in dieser Reihenfolge an. Ein für Ihre Sprache gedachtes Format (die fertigen aus der Liste) verwendet die Reihenfolge und die Namen Ihrer Sprache.

Text, den das Format in Anführungszeichen setzt, etwa „Einheiten“, wird unverändert angezeigt.

## Ein Format auswählen

1. Markieren Sie die Zellen.
2. Verwenden Sie die Gruppe **Zahl** auf der Registerkarte **Start**, oder drücken Sie **Strg+1** (auf dem Mac **Befehl+1**), um **Zellen formatieren** zu öffnen. Das Fenster beginnt auf der Registerkarte **Zahl**.

Das Feld in der Gruppe **Zahl** enthält die gängigen Formate: Standard, Zahl, Währung, Buchhaltung, Kurzes Datum, Langes Datum, Uhrzeit, Prozentsatz, Bruch, Wissenschaftlich, Text und Benutzerdefiniert. **Weitere Zahlenformate** am Ende öffnet das Fenster Zellen formatieren. Die Schaltflächen unter dem Feld sind:

| Schaltfläche | Was sie bewirkt |
| --- | --- |
| Währung | Zeigt die Zahl als Geldbetrag mit dem Währungssymbol Ihrer Sprache an (**Strg+Umschalt+4**). Der Pfeil daneben bietet **Buchhaltungsformat für Zahlen** mit einer Währung zur Auswahl (US-Dollar, Britisches Pfund, Euro, Japanischer Yen, Chinesischer Yuan oder Schweizer Franken), Standard und **Weitere Zahlenformate**. |
| Prozentformat | Multipliziert mit 100 und fügt ein Prozentzeichen ohne Dezimalstellen hinzu (**Strg+Umschalt+5**). Klicken Sie erneut darauf, um es zu entfernen. |
| 1000er-Trennzeichen | Gruppiert die Ziffern in Tausendern, zeigt zwei Dezimalstellen an und richtet die Dezimalzeichen untereinander aus, ohne Symbol. |
| Dezimalstelle hinzufügen, Dezimalstelle entfernen | Fügt eine Dezimalstelle hinzu oder entfernt sie. |
| Standard (im Feld und im Pfeil neben Währung) | Hebt das Format auf: Die Zahl wird so angezeigt, wie sie ist (**Strg+Umschalt+~**). |

## Die Registerkarte Zahl im Fenster Zellen formatieren

Wählen Sie links eine **Kategorie**; rechts erscheinen die Optionen dafür, und das **Beispiel** zeigt, wie die erste markierte Zelle in Ihrer Sprache aussehen wird.

- **Zahl**: Dezimalstellen, ob die Ziffern in Tausendern gruppiert werden, und wie negative Zahlen aussehen (mit Minuszeichen, rot oder in Klammern).
- **Währung** und **Buchhaltung**: Dezimalstellen und ein Symbol. Die Buchhaltung richtet die Symbole und die Dezimalzeichen untereinander aus.
- **Datum** und **Uhrzeit**: eine Liste fertiger Formate, mit der Reihenfolge und den Namen Ihrer Sprache.
- **Prozentsatz**, **Wissenschaftlich**: Dezimalstellen.
- **Bruch**: fertige Formate wie „bis zu einer Ziffer“ (1/4), „bis zu zwei Ziffern“ (21/25), „als Viertel“ und „als Sechzehntel“.
- **Sonderformat**: fertige Formate für eine Postleitzahl, eine Postleitzahl plus 4, eine Telefonnummer und eine Sozialversicherungsnummer.
- **Text**: Die Zelle enthält, was Sie als Text eingeben, sodass eine Zahl wie 00123 ihre Nullen behält.
- **Benutzerdefiniert**: Geben Sie einen eigenen Code ein, oder beginnen Sie mit einem aus der Liste (den gängigen Codes und denen, die die Arbeitsmappe bereits verwendet).

Der **Code** ist der kurze Text, der JeffCrunch sagt, wie eine Zahl geschrieben wird. Beispiel: **#.##0,00** bedeutet „Ziffern in Tausendern gruppieren und immer zwei Dezimalstellen anzeigen“, sodass 1234,5 als 1.234,50 erscheint. Jede **0** ist eine Ziffer, die immer angezeigt wird, jedes **#** eine Ziffer, die nur angezeigt wird, wenn sie nötig ist, und **%** multipliziert mit 100. Ein Code kann bis zu vier Abschnitte haben, durch Semikolons getrennt: für positive Zahlen, negative Zahlen, null und Text.

## Was erhalten bleibt

Der Code wird in neutraler Form in der Arbeitsmappe gespeichert, damit die Arbeitsmappe auf jedem Computer richtig angezeigt wird. Die Formatierung lässt sich rückgängig machen, und eine Arbeitsmappe, die bereits ein Format verwendet, behält es genau bei: Ein Format, das in der Liste steht, wird wiedergefunden, und keines wird doppelt angelegt.
